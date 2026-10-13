/* The library the tests exclude from analysis.
 *
 * Every function here reaches global data, so on ppc64 each call needs a
 * correct r2 (TOC pointer), and a call in from another object needs Dyninst to
 * know this object's TOC base.  Built shared, ppc64le gives each function the
 * ABI v2 dual entry point: a global entry that establishes r2 and a local
 * entry 8 bytes later.
 */
int toc_global = 0x5EED;
static int toc_table[4] = { 11, 22, 33, 44 };


int toc_get(void)
{
    return toc_global;
}
int toc_index(int i)
{
    return toc_table[i & 3];
}
int toc_sum(void)
{
    return toc_table[0]+toc_table[1]+toc_table[2]+toc_table[3];
}
void toc_bump(void)
{
    toc_global++;
}
int toc_marker(void)
{
    return 1;
}

/* A call that stays inside this library, for the ppc64 global/local entry
   pair.  ELFv2 gives a function that uses the TOC two entry points -- a global
   one that establishes r2 from r12, and a local one eight bytes later that
   skips it -- and places the symbol on the global one.  A call the linker can
   resolve within the object is aimed at the local entry, which no symbol
   names.  Hidden visibility is what lets the linker resolve it: an exported
   function is interposable, so a call to one goes through a PLT stub and
   arrives at the global entry instead, which is not the case under test.
   toc_leaf reaches global data so that it needs r2 and so has both entry
   points at all. */
__attribute__((visibility("hidden"), noinline))
int toc_leaf(int x)
{
    return toc_global + toc_table[x & 3];
}

/* The addition keeps this from becoming a tail call, which would leave a
   branch where the test wants a call site. */
int toc_chain(int x)
{
    return toc_leaf(x) + toc_global;
}

/* Bulk, so that excluding this library removes a measurable number of basic
   blocks rather than a handful.  Each function below is several blocks, and
   there are two hundred of them. */
#define TOC_FILLER(n)                                                         \
    int toc_filler_##n(int x)                                                 \
    {                                                                         \
        int r = toc_global;                                                   \
        if (x & 1)                                                            \
            r += toc_table[0];                                                \
        else                                                                  \
            r -= toc_table[1];                                                \
        for (int i = 0; i < (x & 7); i++)                                     \
            r += toc_table[i & 3];                                            \
        if (r > 100)                                                          \
            r = 100;                                                          \
        return r;                                                             \
    }

#define TOC_FILLER_10(p)                                                      \
    TOC_FILLER(p##0) TOC_FILLER(p##1) TOC_FILLER(p##2) TOC_FILLER(p##3)       \
    TOC_FILLER(p##4) TOC_FILLER(p##5) TOC_FILLER(p##6) TOC_FILLER(p##7)       \
    TOC_FILLER(p##8) TOC_FILLER(p##9)

#define TOC_FILLER_100(p)                                                     \
    TOC_FILLER_10(p##0) TOC_FILLER_10(p##1) TOC_FILLER_10(p##2)               \
    TOC_FILLER_10(p##3) TOC_FILLER_10(p##4) TOC_FILLER_10(p##5)               \
    TOC_FILLER_10(p##6) TOC_FILLER_10(p##7) TOC_FILLER_10(p##8)               \
    TOC_FILLER_10(p##9)

TOC_FILLER_100(1)
TOC_FILLER_100(2)
