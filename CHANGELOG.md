## 1.0.0

- **nd-mob is now an installable library rather than a build artifact of
  the engine.** It builds and installs exactly one file,
  `lib/libnd-mob.so`, following the same layout as `axil-tty` and
  `axil-auth`, and the same layout `nd-core` was converted to first.
  Previously `make` produced a `mob.so` named by the engine's `mods.load`
  and installed nothing. There is no `lib/nd-mob.so` symlink: `mods.load`
  names this module `libnd-mob`, the installed filename, and
  `module_load_path()` only appends `.so`. It installs no header because it
  exports no API — the one symbol it defines (`on_spawn`) is discovered by
  the engine, not called by another module.

- **The link line is libxylem alone.** `LDLIBS := -lxylem`; the engine is not
  linked. `NEEDED` is `libxylem.so` and `libc.so.6`.

- **There was no `mod_open` with a distinct body** — `xy_install` is the old
  `mod_install` verbatim. No tables are opened here; the module reads
  nd-fight's skeletons, nd-plant's tiles and nd-race's races.

- **Dropped the `nd-mod.mk` dependency.** `nd-mod.mk` has now been deleted.
