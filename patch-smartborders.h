static int smartborders = 0;

static inline void
patch_smartborders_config_parse()
{
        smartborders = dscm_alist_get_int(config, "smart-borders");
}
