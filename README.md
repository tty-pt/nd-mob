# axil-nd-mob

`nd-mob` for [axil-nd](../axil-nd), ported from SIC to libxylem.

Owns mobiles: spawning entities onto generated tiles. It implements exactly
one hook, `on_spawn` — reading the tile (`plant_tile_t`), the race and the
fight skeleton to place a mob — and keeps no tables of its own.

## Install

```sh
make install
```

Installs one file:

```
lib/libnd-mob.so
```

There is deliberately no `lib/nd-mob.so` symlink (see `axil-nd-wts` for
why: `mods.load` names the installed filename, and the OpenBSD packing list
never lists a symlink).

Also packaged for deb, apk, rpm, brew and openbsd from a `v*` tag.

It installs no header because it exports no API — the one symbol it defines
is discovered by the engine, not called by another module.

## Build from source

```sh
make
```

Needs [libxylem](https://github.com/tty-pt/libxylem) and the engine's game
API, `<nd/xy.h>`, plus the sibling types it reads (`<nd/fight.h>`,
`<nd/plant.h>`, `<nd/race.h>`) — from checkouts beside this repo or from
installed packages:

```sh
git clone https://github.com/tty-pt/nd-mob && cd nd-mob
git clone https://github.com/tty-pt/axil-nd ../axil-nd
git clone https://github.com/tty-pt/nd-fight ../axil-nd-fight
git clone https://github.com/tty-pt/nd-plant ../axil-nd-plant
git clone https://github.com/tty-pt/nd-race ../axil-nd-race
make
```

Both the checkout `-I` flags and the installed-package paths are on the
command line at once (see `Makefile`), and a missing `-I` is ignored, so the
same command works either way. CI names the deps explicitly
(`axil-nd,libxylem,nd-fight,nd-plant,nd-race`).

## What it does

* `on_spawn` reads the tile, picks the species by spawn density, and places
  the mob. That is the whole module.

## Packaged Assets

- **Art (`art/`)**: Monster, creature, and NPC entity graphics stored in `art/entity/`.

## Testing

There is no `test.sh` here. Behaviour is asserted by the engine's own suite:

```sh
cd ../axil-nd
make && ./test.sh
```

## Notes from the port

* `SIC_DEF` → `XY_IMPL`, `mod_install` → `xy_install`. There was no
  `mod_open` with a distinct body; `xy_install` is the old `mod_install`
  verbatim.
* The link line is libxylem alone. `NEEDED` is `libxylem.so` and `libc.so.6`.

## License

BSD 2-Clause, carried over from `tty-pt/nd-mob`. See `LICENSE`.
