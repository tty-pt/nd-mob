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

XY_MODULE_API void
xy_install(void)
{
	unsigned wt_peck = (unsigned)nd_put(HD_WTS, NULL, "peck");
	unsigned wt_bite = (unsigned)nd_put(HD_WTS, NULL, "bite");
	unsigned bird = race_add("bird", 0, 0, 2, 0, 1, 0, wt_peck);
	unsigned fish = race_add("fish", 0, 2, 0, 1, 0, 0, wt_bite);
	mob_skel_add("goldfish", 2, ELM_WATER, water, 4, 0, 0, 0, fish);
	mob_skel_add("salmon", 6, ELM_WATER, water, 4, 0, 0, 0, fish);
	mob_skel_add("tuna", 13, ELM_WATER, water, 6, 0, 0, 0, fish);
	mob_skel_add("koifish", 3, ELM_WATER, water, 1, 0, 0, 0, fish);
	mob_skel_add("dolphin", 1, ELM_WATER, water, 1, 0, 0, 0, fish);
	mob_skel_add("shark", 7, ELM_WATER, water, 13, 40, 0x1f, FF_AGGRO, fish);
	/* .y = 13, .lvl = 40, .lvl_v = 0x1f, */
	mob_skel_add("moonfish", 0, ELM_WATER, water, 14, 0, 0, FF_AGGRO, fish);
	mob_skel_add("rainbowfish", 6, ELM_WATER, water, 14, 0, 0, FF_AGGRO, fish);
	mob_skel_add("icebird", 1, ELM_WATER, (
			 (1 << BIOME_PERMANENT_ICE)
			 | (1 << BIOME_TUNDRA)
			 | (1 << BIOME_TUNDRA2)
			 | (1 << BIOME_TUNDRA3)
			 | (1 << BIOME_TUNDRA4)
			 | (1 << BIOME_COLD_DESERT)
			), 14, 0, 0, FF_AGGRO, bird);
	mob_skel_add("parrot", 6, ELM_AIR, (1 << BIOME_TEMPERATE_RAINFOREST), 4, 0, 0, FF_AGGRO, bird);
	mob_skel_add("swallow", 9, ELM_AIR, common, 4, 0, 0, 0, bird);
	mob_skel_add("woodpecker", 7, ELM_AIR, common, 2, 0, 0, 0, bird);
	mob_skel_add("sparrow", 15, ELM_AIR, common, 3, 0, 0, 0, bird);
	mob_skel_add("owl", 9, ELM_AIR, common, 7, 0, 0, 0, bird);
}