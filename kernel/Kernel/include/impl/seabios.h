#pragma once

#define GET_LOW(var)            (var)
#define SET_LOW(var, val)       do { (var) = (val); } while (0)
#define LOWFLAT2LOW(var) (var)

#define GET_LOWFLAT(var) GET_LOW(*LOWFLAT2LOW(&(var)))
#define SET_LOWFLAT(var, val) SET_LOW(*LOWFLAT2LOW(&(var)), (val))