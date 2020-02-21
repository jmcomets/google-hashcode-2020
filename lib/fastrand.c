static unsigned int g_seed;

inline void fast_srand(int seed)
{
    g_seed = seed;
}

inline int fast_rand()
{
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16)&0x7FFF;
}
