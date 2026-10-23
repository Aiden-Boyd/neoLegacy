# End portal projection candidate

Branch: `fix/end-portal-projection`, based on the flight/head/inventory candidate
at cca254b. Existing End blocks, sponge transfer fixes, and flight/head fixes remain.

## Source defects corrected

The Windows renderer transformed eye-space texture planes as ordinary vectors
using only the inverse model-view matrix. Plane coefficients require the inverse
transpose. With a rotated camera, the old code did not preserve the plane equation,
so the projected texture denominator could vary across an otherwise flat portal.
This is a plausible cause of the angle-dependent streaks in the reported images.

`UpdateTexGenState` also unmapped the projection constant buffer before copying
the matrices into it. That writes through a pointer whose mapping has already
ended. The upload now copies before Unmap and returns safely if Map fails.

This changes the Windows compatibility renderer, not portal geometry, textures,
blend layers, teleport behavior, or world generation. It preserves the existing
layered portal effect rather than replacing it with a flat animated texture.

## Automated evidence

`python tests/portal_projection_regression.py` extracts and compiles the actual
production transform and upload methods with a deterministic math/D3D fixture.
It verifies plane-equation preservation across yaw/pitch combinations and sample
points, clearing the inactive texgen plane, uploaded buffer contents and mapping
failure handling. Restoring the original transform or upload order independently
fails the corresponding check. The fixture does not run a GPU or DXVK.

All earlier regression suites also run in CI, followed by a full Windows client
build. Compilation and fixture results do not establish visual correctness.

## In-game acceptance on Bazzite

Extract separately and use the same Steam/Proton/controller setup as before.
Use the same portal and viewpoints from the reported screenshots if possible.

1. Walk around all four sides and corners, slowly sweeping yaw and pitch.
2. Look almost straight down, then at increasingly shallow angles. The layered
   pattern should remain coherent without long radial streaks or flashes.
3. Walk, crouch and fly above the portal at different heights. Also check third
   person. Watch the join between adjacent portal blocks for new seams.
4. Test another portal at a different world position and altitude, including
   negative coordinates. Check the return portal in the End as well.
5. Recheck entering/exiting the portal, normal world rendering, inventory preview,
   equipped heads, furnace transfer and saving. The underlying texgen change can
   affect any other renderer using eye-space generated texture coordinates.

Keep the previous test build available. If distortion remains, capture the exact
camera position/pitch and whether it happens in first or third person. The image
symptom has not been reproduced here on the user's GPU/Proton setup.

## API references

- https://learn.microsoft.com/en-us/windows/win32/api/directxmath/nf-directxmath-xmplanetransform
- https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-unmap
