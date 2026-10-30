using System.Text;

static class ServerList
{
    record Entry(byte[] Address, ushort Port, byte[] Name);
    const string Prefix = "[Join Code] ";
    public static void Add(string directory, string code, int port)
    {
        var path = Path.Combine(directory, "servers.db");
        var entries = new List<Entry>();
        byte[]? original = null;
        if (File.Exists(path))
        {
            original = File.ReadAllBytes(path);
            using var memory = new MemoryStream(original);
            using var reader = new BinaryReader(memory, Encoding.UTF8);
            if (Encoding.ASCII.GetString(reader.ReadBytes(4)) != "MCSV" || reader.ReadUInt32() != 1)
                throw new IOException("Unsupported server list; it was not modified.");
            uint count = reader.ReadUInt32();
            if (count > 4096) throw new IOException("Invalid server list; it was not modified.");
            for (int i = 0; i < count; i++)
            {
                byte[] ReadField()
                {
                    int length = reader.ReadUInt16();
                    if (length > 256) throw new IOException("Invalid server-list field.");
                    var bytes = reader.ReadBytes(length);
                    if (bytes.Length != length) throw new EndOfStreamException();
                    return bytes;
                }
                var address = ReadField(); var oldPort = reader.ReadUInt16(); var name = ReadField();
                if (!Encoding.UTF8.GetString(name).StartsWith(Prefix)) entries.Add(new(address, oldPort, name));
            }
            if (memory.Position != memory.Length) throw new IOException("Unexpected server-list data; it was not modified.");
        }
        entries.Add(new(Encoding.ASCII.GetBytes("127.0.0.1"), checked((ushort)port), Encoding.UTF8.GetBytes(Prefix + code)));
        var temporary = path + ".join-tmp";
        try
        {
            using (var file = new FileStream(temporary, FileMode.Create, FileAccess.Write, FileShare.None))
            {
                using var writer = new BinaryWriter(file, Encoding.UTF8, leaveOpen: true);
                writer.Write(Encoding.ASCII.GetBytes("MCSV")); writer.Write((uint)1); writer.Write((uint)entries.Count);
                foreach (var entry in entries)
                {
                    writer.Write((ushort)entry.Address.Length); writer.Write(entry.Address); writer.Write(entry.Port);
                    writer.Write((ushort)entry.Name.Length); writer.Write(entry.Name);
                }
                writer.Flush(); file.Flush(flushToDisk: true);
            }
            if (original != null) File.WriteAllBytes(path + ".before-join.bak", original);
            File.Move(temporary, path, overwrite: true);
        }
        finally { if (File.Exists(temporary)) File.Delete(temporary); }
    }
}
