#include <stdlib.h>

#include <nd/nd.h>
#include <nd/fight.h>
#include <nd/plant.h>

enum wt {
	WT_BITE,
	WT_PECK,
};

unsigned water = (1 << BIOME_WATER);
unsigned common = ((1 << BIOME_SHRUBLAND)
		| (1 << BIOME_CONIFEROUS_FOREST)
		| (1 << BIOME_BOREAL_FOREST)
		| (1 << BIOME_TEMPERATE_GRASSLAND)
		| (1 << BIOME_WOODLAND)
		| (1 << BIOME_TEMPERATE_SEASONAL_FOREST)
		| (1 << BIOME_TEMPERATE_RAINFOREST)
		);

char *wts[] = {
	[WT_PECK] = "peck",
	[WT_BITE] = "bite",
};

unsigned wt_refs[2];
unsigned mob_refs[32], mob_max = 0;
unsigned birds[32];

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

int
on_spawn(unsigned player_ref __attribute__((unused)),
		unsigned where_ref, struct bio bio,
		uint64_t v __attribute__((unused)))
{
	unsigned pdn = ((struct plant_data *) bio.raw)->n;
	for (unsigned mid = 0; mid < mob_max; mid++)
		mob_add(mid, where_ref, bio.bio_idx, pdn);
	return 0;
}

static inline unsigned mob_skel_add(
		char *name,
		unsigned max_art,
		unsigned wt,
		unsigned element,
		unsigned biomes,
		unsigned y,
		unsigned char lvl,
		unsigned char lvl_v,
		unsigned char flags)
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
	unsigned skid = nd_put(HD_SKEL, NULL, &skel);
	call_fighter_skel_add(skid, wt, lvl, lvl_v, flags);
	birds[mob_max] = wt == wt_refs[WT_PECK];
	mob_refs[mob_max] = skid;
	mob_max++;
	return skid;
}

void
mob_open(void) {
	// when opening dbs, we still need this:
	nd_get(HD_RWTS, &wt_refs[WT_PECK], "peck");
}

void
mod_install(void) {
	mob_open();

	if (nd_get(HD_RWTS, &wt_refs[WT_BITE], "bite"))
		wt_refs[WT_BITE] = nd_put(HD_WTS, NULL, "bite");

	if (nd_get(HD_RWTS, &wt_refs[WT_PECK], "peck"))
		wt_refs[WT_PECK] = nd_put(HD_WTS, NULL, "peck");

	mob_skel_add("goldfish", 2, WT_BITE, ELM_WATER, water, 4, 0, 0, 0);
	mob_skel_add("salmon", 6, WT_BITE, ELM_WATER, water, 4, 0, 0, 0);
	mob_skel_add("tuna", 13, WT_BITE, ELM_WATER, water, 6, 0, 0, 0);
	mob_skel_add("koifish", 3, WT_BITE, ELM_WATER, water, 1, 0, 0, 0);
	mob_skel_add("dolphin", 1, WT_BITE, ELM_WATER, water, 1, 0, 0, 0);
	mob_skel_add("shark", 7, WT_BITE, ELM_WATER, water, 13, 40, 0x1f, FF_AGGRO);
	/* .y = 13, .lvl = 40, .lvl_v = 0x1f, */
	mob_skel_add("moonfish", 0, WT_BITE, ELM_WATER, water, 14, 0, 0, FF_AGGRO);
	mob_skel_add("rainbowfish", 6, WT_BITE, ELM_WATER, water, 14, 0, 0, FF_AGGRO);
	mob_skel_add("icebird", 1, WT_PECK, ELM_WATER, (
			 (1 << BIOME_PERMANENT_ICE)
			 | (1 << BIOME_TUNDRA)
			 | (1 << BIOME_TUNDRA2)
			 | (1 << BIOME_TUNDRA3)
			 | (1 << BIOME_TUNDRA4)
			 | (1 << BIOME_COLD_DESERT)
			), 14, 0, 0, FF_AGGRO);
	mob_skel_add("parrot", 6, WT_PECK, ELM_WATER, (1 << BIOME_TEMPERATE_RAINFOREST), 4, 0, 0, FF_AGGRO);
	mob_skel_add("swallow", 9, WT_PECK, ELM_AIR, common, 4, 0, 0, 0);
	mob_skel_add("woodpecker", 7, WT_PECK, ELM_AIR, common, 2, 0, 0, 0);
	mob_skel_add("sparrow", 15, WT_PECK, ELM_AIR, common, 3, 0, 0, 0);
	mob_skel_add("owl", 9, WT_PECK, ELM_AIR, common, 7, 0, 0, 0);
}
