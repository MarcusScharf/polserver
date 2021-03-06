/** @file
 *
 * @par History
 *
 * @note SOLIDX2_FILLER_SIZE bytes are at the beginning of the solidx2.dat file,
 * so that something referencing the first element won't reference offset 0.
 */


#ifndef PLIB_MAPBLOB_H
#define PLIB_MAPBLOB_H

#define SOLIDX_X_SIZE 16
#define SOLIDX_X_SHIFT 4
#define SOLIDX_X_CELLMASK 0xF

#define SOLIDX_Y_SIZE 16
#define SOLIDX_Y_SHIFT 4
#define SOLIDX_Y_CELLMASK 0xF
namespace Pol
{
namespace Plib
{
struct SOLIDX1_ELEM
{
  unsigned int offset;
};

const unsigned SOLIDX2_FILLER_SIZE = 4;

struct SOLIDX2_ELEM
{
  unsigned int baseindex;
  unsigned short addindex[SOLIDX_X_SIZE][SOLIDX_Y_SIZE];
};
static_assert( sizeof( SOLIDX2_ELEM ) == 516, "size missmatch" );

struct SOLIDS_ELEM
{
  signed char z;
  unsigned char height;
  unsigned char flags;
};
static_assert( sizeof( SOLIDS_ELEM ) == 3, "size missmatch" );
}
}
#endif
