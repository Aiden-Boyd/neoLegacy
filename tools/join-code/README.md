# Legacy Join Codes — experimental companion and Cloudflare relay

This adds genuine outbound relay transport to the old Steam/Proton baseline.
It is not an encoded public IP address and does not require players to forward
ports. It is a separate companion, not yet a built-in in-game Host/Join button.
The old game executable and gameplay code remain unchanged.

## Current status

- Local Cloudflare-runtime protocol tests pass: invalid/unknown codes, duplicate
  hosts, stream authentication, byte preservation, ACKs, host departure and an
  enforced eight-frame window.
- The .NET companion passes an end-to-end four-client, 4 MiB TCP echo test through
  the local Worker runtime. Server-list replacement and corrupt-list preservation
  are tested too.
- Public deployment, a real two-player game session, and Bazzite acceptance have
  not been verified. Do not label this ready for players until those pass.
- A live relay URL is required. No fake or shared third-party endpoint is bundled.

## Deploy once to your Cloudflare account

From this directory's `relay` folder:

```sh
npm ci
npx wrangler login
npm run deploy
```

The config creates a SQLite-backed Durable Object namespace. Review the resource
creation in your own account. Cloudflare's free plan supports this backend, with
usage limits; this is not a promise of unlimited free traffic. No VPS is required.
See https://developers.cloudflare.com/durable-objects/platform/limits/ and
https://developers.cloudflare.com/durable-objects/platform/pricing/.

Use the deployed HTTPS hostname with the `wss://` scheme in `relay.json`, beside
the companion and `Minecraft.Client.exe`:

```json
{ "relay": "wss://YOUR-WORKER.YOUR-SUBDOMAIN.workers.dev" }
```

The relay forwards only the paired game byte streams. Host registration uses a
random bearer token; guests possess the random 10-character room code. Treat a
shared code as access to the hosted world. Rooms disappear when the host leaves.
Up to eight guest TCP connections are allowed per room. The relay has bounded
16 KiB frames and an eight-frame acknowledgement window in each direction.
There is no public room listing. Codes/tokens are not logged by this source.
Public abuse controls and account quotas still need operational review.

## Play with the companion

Put the Windows `LegacyJoin.exe`, or Linux `LegacyJoin`, directly beside the old
`Minecraft.Client.exe`, together with `relay.json`. All players use the same relay
URL and the same old game build.

Host:

1. Run the companion and choose **Host**.
2. Windows opens the game; on Bazzite, launch it through your existing Steam/Proton
   shortcut from the same game directory.
3. Create/load an online-enabled world. After the local listener and relay connect,
   the companion displays a code such as `ABCDE-FGHJK`.
4. Share that code and keep both the world and companion running.

Guest:

1. Close the game, run the companion, choose **Join**, and enter the code.
2. The companion creates a loopback proxy and adds a `[Join Code]` server entry.
3. Windows opens the game; on Bazzite, launch through Steam/Proton.
4. In **Join Game**, select the `[Join Code]` entry. No public IP or port is typed.
5. Keep the companion open. Ctrl+C stops the relay session; it does not kill the game.

The companion understands only the old version-1 `MCSV` server list. It preserves
ordinary entries, backs up the original, and refuses unsupported/corrupt formats.
It never changes world saves. New helper sessions replace prior helper entries
because their random loopback port is no longer valid. The helper does not add
Java/Bedrock/Realms compatibility or Steam invites.

## Build and verify

```sh
dotnet build helper/LegacyJoin.csproj
cd relay
npm ci
npm run dev -- --port 8787
```

In another terminal on the same machine:

```sh
cd relay
npm test
dotnet run --project ../helper/LegacyJoin.csproj -- self-test ws://127.0.0.1:8787
```

Public URLs must use WSS. Unencrypted WS is accepted only on loopback for tests.
CI publishes self-contained Windows x64 and Linux x64 companions, plus relay
source and test logs. No paid resource or public deployment is triggered by CI.

Before player use, verify a disposable world across two houses: join, place/break
blocks, save, disconnect/rejoin, host restart, and stop sharing. Then measure chunk
loading, latency and traffic/quota usage. The local echo test cannot prove those.
