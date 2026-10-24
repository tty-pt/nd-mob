/* src/libnd-mob.c — nd-mob, ported to libxylem.
 *
 * Owns mobs: the fish and bird skeletons, their races and fighter stats, and
 * spawning them from map tiles. A pure consumer -- it implements one engine
 * hook (on_spawn) and calls other modules' services, shipping no header.
 *
 * Original: tty-pt/nd-mob @ 112 lines main.c, from the nd-basics
 * superproject.
 *
 * This TU XY_IMPLs on_spawn only. It calls fighter_skel_add (nd-fight) and
 * race_set / race_add (nd-race), and reads plant_tile_t (nd-plant). It
 * implements nothing from another module's header, so no *_IMPL guard is
 * needed.
 *
 * No mod_open existed; xy_install is the old mod_install verbatim.
 */

#include <ttypt/xy-mod.h>

#include <nd/xy.h>

#include <stdlib.h>
#include <string.h>

#include <nd/fight.h>
#include <nd/plant.h>
#include <nd/race.h>

static unsigned water = (1 << BIOME_WATER);
static unsigned common = ((1 << BIOME_SHRUBLAND)
		| (1 << BIOME_CONIFEROUS_FOREST)
		| (1 << BIOME_BOREAL_FOREST)
		| (1 << BIOME_TEMPERATE_GRASSLAND)
		| (1 << BIOME_WOODLAND)
		| (1 << BIOME_TEMPERATE_SEASONAL_FOREST)
		| (1 << BIOME_TEMPERATE_RAINFOREST)
		);

static unsigned mob_refs[32], mob_max = 0;
static unsigned birds[32];

/* XY_IMPL both defines the function and emits the dispatch adapter, so each
 * name gets exactly one, with its body -- no forward declarations.
 *
 * Order matters below: XY_IMPL emits a definition, so a caller has to come
 * after its callee. */

static inline unsigned
mob_add(unsigned mid, unsigned where_ref, enum biome biome, long long pdn) {
	unsigned skel_ref = mob_refs[mid];
	SKEL skel;
	SENT *mob_skel = (SENT *) &skel.data;
	OBJ obj;

	nd_get(HD_SKEL, &skel, &skel_ref);

	if ((birds[mid] && !pdn)
	    // || (!NIGHT_IS && (mob_skel->type == ELM_DARK || mob_skel->type == ELM_VAMP))
	    || random() >= (RAND_MAX >> mob_skel->y))
		return NOTHING;

	if (!((1 << biome) & mob_skel->biomes))
		return NOTHING;

	return object_add(&obj, skel_ref, where_ref, 0);
}

XY_IMPL(int, on_spawn, unsigned, player_ref,
		unsigned, where_ref, struct bio, bio,
		uint64_t, v)
{
	(void) player_ref;
	(void) v;
	unsigned pdn = ((plant_tile_t *) bio.raw)->n;
	for (unsigned mid = 0; mid < mob_max; mid++)
		mob_add(mid, where_ref, bio.bio_idx, pdn);
	return 0;
}

static inline unsigned mob_skel_add(
		char *name,
		unsigned max_art,
		unsigned element,
		unsigned biomes,
		unsigned y,
		unsigned char lvl,
		unsigned char lvl_v,
		unsigned char flags,
		unsigned race_id)
{
	SENT ent_skel = {
		.biomes = biomes,
		.element = element,
		.y = y,
	};

	SKEL skel = {
		.max_art = max_art,
		.type = TYPE_ENTITY,
	};

	strlcpy((char *) skel.name, name, sizeof(skel.name));
	memcpy(skel.data, &ent_skel, sizeof(ent_skel));
	unsigned skid = (unsigned)nd_put(HD_SKEL, NULL, &skel);
	fighter_skel_add(skid, lvl, lvl_v, flags);
	race_set(skid, race_id);
	mob_refs[mob_max] = skid;
	mob_max++;
	return skid;
}

static unsigned bird_race_id, fish_race_id;

static int
mob_row(const char *const *f, size_t n, void *user)
{
	static const struct { const char *n; unsigned v; } elm[] = {
		{ "spirit", ELM_SPIRIT }, { "fire", ELM_FIRE }, { "water", ELM_WATER },
		{ "air", ELM_AIR }, { "earth", ELM_EARTH }, { "physical", ELM_PHYSICAL },
	};
	(void)user;
	if (n < 9)
		return 0;

	char *name = (char *)f[0];
	unsigned max_art = (unsigned)atoi(f[1]);
	unsigned element = ELM_PHYSICAL;
	for (size_t i = 0; i < sizeof(elm) / sizeof(elm[0]); i++)
		if (!strcmp(elm[i].n, f[2]))
			element = elm[i].v;

	unsigned biomes = 0;
	if (!strcmp(f[3], "water"))
		biomes = water;
	else if (!strcmp(f[3], "common"))
		biomes = common;
	else if (!strcmp(f[3], "cold"))
		biomes = (
			 (1 << BIOME_PERMANENT_ICE)
			 | (1 << BIOME_TUNDRA)
			 | (1 << BIOME_TUNDRA2)
			 | (1 << BIOME_TUNDRA3)
			 | (1 << BIOME_TUNDRA4)
			 | (1 << BIOME_COLD_DESERT)
		);
	else if (!strcmp(f[3], "temperate_rainforest"))
		biomes = (1 << BIOME_TEMPERATE_RAINFOREST);
	else
		biomes = (unsigned)strtoul(f[3], NULL, 0);

	unsigned y = (unsigned)atoi(f[4]);
	unsigned char lvl = (unsigned char)atoi(f[5]);
	unsigned char lvl_v = (unsigned char)strtoul(f[6], NULL, 0);
	unsigned char flags = (!strcmp(f[7], "aggro") || atoi(f[7])) ? FF_AGGRO : 0;
	unsigned race_id = !strcmp(f[8], "bird") ? bird_race_id : fish_race_id;

	mob_skel_add(name, max_art, element, biomes, y, lvl, lvl_v, flags, race_id);
	return 0;
}

XY_MODULE_API void
xy_install(void)
{
	unsigned wt_peck = (unsigned)nd_put(HD_WTS, NULL, "peck");
	unsigned wt_bite = (unsigned)nd_put(HD_WTS, NULL, "bite");
	bird_race_id = race_add("bird", 0, 0, 2, 0, 1, 0, wt_peck);
	fish_race_id = race_add("fish", 0, 2, 0, 1, 0, 0, wt_bite);

	if (nd_table_load("nd-mob", "mobs", mob_row, NULL) < 0)
		WARN("nd-mob: mobs.dsv not found, no mobs loaded\n");
}