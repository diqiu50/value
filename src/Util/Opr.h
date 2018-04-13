////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////
#ifndef Util_Opr_h
#define Util_Opr_h

namespace value
{
enum class Opr
{
	eOpr_Invalid,

	eOpr_gt,			// >
	eOpr_ge,			// >=
	eOpr_eq,			// ==
	eOpr_lt,			// <
	eOpr_le,			// <=
	eOpr_ne,			// !=
	eOpr_an,			// Matching any
	eOpr_like,
	eOpr_not_like,
	eOpr_null,
	eOpr_not_null,
	eOpr_date,
	eOpr_epoch,
	eOpr_range_ge_le,
	eOpr_range_ge_lt,
	eOpr_range_gt_le,
	eOpr_range_gt_lt,
	eOpr_in,
	eOpr_not_in,
	eOpr_distinct,
	eOpr_and,
	eOpr_or,
	eOpr_max
};
}

#endif

