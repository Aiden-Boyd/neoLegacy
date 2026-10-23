# End city implementation

The original template-import checkpoint is preserved on `work/end-city-templates`. Runtime implementation is on `feature/end-cities-shulkers` and includes the preceding gateway candidate.

Read `END_CITY_TESTS.md` for behavior, validation, known parity gaps and in-game acceptance. Source assets and their Git blob hashes remain under `assets/endcity`; `tools/compile_end_city_templates.py` retains the original data, and `tools/generate_end_city_header.py` maps validated palettes to this engine's IDs and metadata.

`EndCityLayout.h` plans geometry without world access. `EndCityFeature.cpp` samples terrain without loading neighbors, places clipped template cells, resolves ship/loot/sentry markers and records completed population chunks. `LevelData` persists that journal and `DerivedLevelData` delegates it to root metadata. This bypasses the generic structure-save framework, which is disabled in this base.

`Shulker.cpp` owns attachment, shell/armor, teleportation and combat. `ShulkerBullet` uses the existing throwable collision path and stores steering/UUID data. The new object packet type and explicit mob tracking path support joining clients. `LivingEntity::travel` handles levitation in place of ordinary gravity while the effect is active.
