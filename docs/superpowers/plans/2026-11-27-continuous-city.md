# Continuous city implementation

1. Freeze the original map for the historical golden render checks; exercise a regression for crossing the old edge. Verify the standalone baseline fails at the east gate before changing the layout.
2. Add the 40x30 map and check connectivity, preserved anchors and outer boundaries. Cache the reverse route field and compare it against independent flood distances for every campaign stage.
3. Resize the actual field-guide map and show the current district in both native world HUDs.
4. Extend the two renderers: bounded road/soft terrain banks, taller facades, shared physical street furnishings; retain original wall-floor intersection and shade cache bounds.
5. Add five optional district observations, actual input-path traversal, coordinate save checks and expanded reference-render comparisons.
6. Run host suite, cartridge budget/build and emulator checks before integration. The local environment cannot clone the repository (DNS unavailable); the standalone map/collision slice was checked red/green locally, full integration must run in repository CI. No claim of measured FPS or new ROM verification until the exact build succeeds.
