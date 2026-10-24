# First mini-game candidate: shovel Tumble and address join codes

Cumulative base: 7fbea59064ebbaef6ea7211146da1ea140e5fa11. Tutorial lessons
remain included. This is a first playable Tumble implementation for testing,
not a port of the complete console mini-game suite. Battle and Glide are not
implemented. The arena is a custom three-floor test arena above the imported
2017 lobby, not one of the original console arenas.

Use the separately supplied Tumble lobby copy. Its requiredGameRules.grf opts
in through MapOptions.dataTag 0x544D4231. Ordinary worlds do not activate the
session and keep their gameplay behavior. The supplied map uses its original
spawn. The reserved arena is 96 blocks east/south, at heights 179 through 208;
the source lobby's highest occupied Y was 127. The engine checks that reserved
space before building and refuses to remove scenery if it is occupied.

## Playing

Copy Tumble_Lobby into neoLegacy/Windows64/GameHDD, then select its .ms save.
Two connected players automatically start preparation and a 15-second countdown.
The host can type /tumble practice for a solo check, /tumble stop to pause rounds,
and /tumble start to resume automatic multiplayer rounds. Commands are available
in the marked lobby without enabling general cheats. Only the host/operator can
start/stop rounds; normal players can type /tumble for help.

Break snow under opponents with the supplied shovel. Three 21x21 floors sit at
Y=200,194,188. Crossing Y=184 or the arena bounds eliminates a participant.
The last survivor wins; all eliminations are applied before checking the result,
so simultaneous final falls draw. The round limit is three minutes. First to
three wins takes a match. Results last eight seconds, then the lobby/countdown
cycle resumes and snow is restored. Late arrivals and eliminated players use a
protected viewing deck. Up to eight players enter each round. A disconnected
player is eliminated; reconnecting joins as a viewer until the next round.

The marked world protects block placement, ordinary mining, player damage,
entity interactions and item dropping. Lobby doors/buttons/levers still work.
Inventories, health, effects and movement privileges are reset for each role;
use a dedicated disposable lobby copy, not a survival world. The mini-game state
and scores reset on server restart. The build is not claiming console spectator
mode, all original lobby puzzles, map voting, custom scoring HUD or network
playtest acceptance.

## Joining with codes

Use /joincode <reachable IPv4> [port] in chat to generate an LCE1 code. The
selected server's details also show its code when it has a numeric IPv4 address.
Paste that code into the Server Address or Join Code field in Add Server (or
Edit Server). A valid code supplies both address and port; then enter a name.
Existing hostname/IP entry still works. Codes use a checksum to reject typos.

This first format encodes an IPv4 address and TCP port. It is not a private room
identifier, password, Steam invite, NAT traversal or VPS relay. LAN addresses
work only where reachable; internet hosting still needs a reachable endpoint.
A later VPS service can supply short room IDs/relay routing, but no backend has
been deployed by this change. The transport remains the existing Win64 TCP
implementation (default port 25565), usable through the existing Steam/Proton
setup. Do not assume cross-version clients will be compatible.

## Import compatibility

The source lobby uses v11 compressed chunks, namespaced entity IDs and text item
IDs. Conversion retains both 128-high sections, block metadata, light, biomes,
height maps and raw dynamic NBT (including duplicate attribute fields), then
writes native legacy NBT regions. Entity/tile aliases cover the supplied lobby;
item loading accepts 1.12.2 names, unqualified names and numeric strings as well
as old numeric tags. Unknown IDs safely reject the item instead of indexing the
item table out of range. Numeric NBT float/double access now coerces numeric tags,
so mixed numeric attribute values are read as values rather than cast layouts.
The registry table comes from the primary 1.12.2 Block.java/Item.java sources,
recorded by blob hash in tests/import_registry_sources.json.

Known decoration gap: the supplied beds are all red, matching the native red bed,
but their modern bed entities are not implemented. Flower-pot entities also
remain unsupported and plants may render empty or be lost on a native save.
The original archive and converted source data remain available; keep them.
Modern skull profiles, hanging-entity orientation, equipment and item displays
still need actual visual checks. The package is a compatibility candidate, not
a claim that every modern NBT feature has complete parity.

## Checks and acceptance

python tests/mini_games_regression.py compiles the production round state,
server controller, address codec and import name lookup. It covers protected
ordinary worlds, preparation failures, role transitions, late joins, simultaneous
falls, disconnect wins, reset, practice, host permissions, match scoring, timeout,
round caps, endpoint boundaries and checksum errors. import_nbt_regression.py
executes production item loading and numeric coercion. Existing equipment tests
still exercise ordinary world use with mini-games disabled. CI builds the whole
Windows64 client and runs all cumulative regressions.

In-game: test solo practice, two local players, two machines over LAN, disconnect
and rejoin mid-round, simultaneous falls, a full match, reset/restart, lobby
protection, controller menus and code entry. Confirm that a normal survival world
and the activated tutorial world still behave normally. GPU output, actual lobby
loading, and real network gameplay have not been tested here.
