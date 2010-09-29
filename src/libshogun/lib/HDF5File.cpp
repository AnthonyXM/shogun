/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2010 Soeren Sonnenburg
 * Copyright (C) 2010 Berlin Institute of Technology
 */

#include <shogun/lib/config.h>

#ifdef HAVE_HDF5
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>

#include "lib/HDF5File.h"

#include "features/StringFeatures.h"
#include "features/SparseFeatures.h"

using namespace shogun;

CHDF5File::CHDF5File(char* fname, char rw, const char* name) : CFile()
{
	get_boolean_type();
	H5Eset_auto(NULL, NULL);

	if (name)
		set_variable_name(name);

	switch (rw)
	{
		case 'r':
			h5file = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);
			break;
		case 'w':
			h5file = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
			break;
		case 'a':
			h5file = H5Fopen(fname, H5F_ACC_RDWR, H5P_DEFAULT);
			if (h5file <0)
				h5file = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
			break;
		default:
			SG_ERROR("unknown mode '%c'\n", rw);
	};

	if (h5file<0)
		SG_ERROR("Could not open file '%s'\n", fname);
}

CHDF5File::~CHDF5File()
{
	H5Fclose(h5file);
}

#define GET_VECTOR(fname, sg_type, datatype)										\
void CHDF5File::fname(sg_type*& vec, int32_t& len)									\
{																					\
	if (!h5file)																	\
		SG_ERROR("File invalid.\n");												\
																					\
	int32_t* dims;																	\
	int32_t ndims;																	\
	int64_t nelements;																\
	hid_t dataset = H5Dopen(h5file, variable_name);									\
	if (dataset<0)																	\
		SG_ERROR("Error opening data set\n");										\
	hid_t dtype = H5Dget_type(dataset);												\
	H5T_class_t t_class=H5Tget_class(dtype);										\
	TSGDataType t datatype; hid_t h5_type=get_compatible_type(t_class, &t);         \
	if (h5_type==-1)																\
	{																				\
		H5Dclose(dataset);															\
		SG_INFO("No compatible datatype found\n");									\
	}																				\
	get_dims(dataset, dims, ndims, nelements);										\
	if (!((ndims==2 && dims[0]==nelements && dims[1]==1) ||							\
			(ndims==2 && dims[0]==1 && dims[1]==nelements) ||						\
			(ndims==1 && dims[0]==nelements)))										\
		SG_ERROR("Error not a 1-dimensional vector (ndims=%d, dims[0]=%d)\n", ndims, dims[0]);	\
	vec=new sg_type[nelements];														\
	len=nelements;																	\
	herr_t status = H5Dread(dataset, h5_type, H5S_ALL, 								\
			H5S_ALL, H5P_DEFAULT, vec);												\
	H5Dclose(dataset);																\
	H5Tclose(dtype);																\
	delete[] dims;																	\
	if (status<0)																	\
	{																				\
		delete[] vec;																\
		SG_ERROR("Error reading dataset\n");										\
	}																				\
}

GET_VECTOR(get_bool_vector, bool, (CT_VECTOR, PT_BOOL))
GET_VECTOR(get_byte_vector, uint8_t, (CT_VECTOR, PT_CHAR))
GET_VECTOR(get_char_vector, char, (CT_VECTOR, PT_CHAR))
GET_VECTOR(get_int_vector, int32_t, (CT_VECTOR, PT_INT32))
GET_VECTOR(get_shortreal_vector, float32_t, (CT_VECTOR, PT_FLOAT32))
GET_VECTOR(get_real_vector, float64_t, (CT_VECTOR, PT_FLOAT64))
GET_VECTOR(get_short_vector, int16_t, (CT_VECTOR, PT_INT16))
GET_VECTOR(get_word_vector, uint16_t, (CT_VECTOR, PT_INT16))
#undef GET_VECTOR

#define GET_MATRIX(fname, sg_type, datatype)										\
void CHDF5File::fname(sg_type*& matrix, int32_t& num_feat, int32_t& num_vec)		\
{																					\
	if (!h5file)																	\
		SG_ERROR("File invalid.\n");												\
																					\
	int32_t* dims;																	\
	int32_t ndims;																	\
	int64_t nelements;																\
	hid_t dataset = H5Dopen(h5file, variable_name);									\
	if (dataset<0)																	\
		SG_ERROR("Error opening data set\n");										\
	hid_t dtype = H5Dget_type(dataset);												\
	H5T_class_t t_class=H5Tget_class(dtype);										\
	TSGDataType t datatype; hid_t h5_type=get_compatible_type(t_class, &t);	        \
	if (h5_type==-1)																\
	{																				\
		H5Dclose(dataset);															\
		SG_INFO("No compatible datatype found\n");									\
	}																				\
	get_dims(dataset, dims, ndims, nelements);										\
	if (ndims!=2)																	\
		SG_ERROR("Error not a 2-dimensional matrix\n");								\
	matrix=new sg_type[nelements];													\
	num_feat=dims[0];																\
	num_vec=dims[1];																\
	herr_t status = H5Dread(dataset, h5_type, H5S_ALL, 								\
			H5S_ALL, H5P_DEFAULT, matrix);											\
	H5Dclose(dataset);																\
	H5Tclose(dtype);																\
	delete[] dims;																	\
	if (status<0)																	\
	{																				\
		delete[] matrix;															\
		SG_ERROR("Error reading dataset\n");										\
	}																				\
}

GET_MATRIX(get_bool_matrix, bool, (CT_MATRIX, PT_BOOL))
GET_MATRIX(get_char_matrix, char, (CT_MATRIX, PT_CHAR))
GET_MATRIX(get_byte_matrix, uint8_t, (CT_MATRIX, PT_CHAR))
GET_MATRIX(get_int_matrix, int32_t, (CT_MATRIX, PT_INT32))
GET_MATRIX(get_uint_matrix, uint32_t, (CT_MATRIX, PT_INT32))
GET_MATRIX(get_long_matrix, int64_t, (CT_MATRIX, PT_INT64))
GET_MATRIX(get_ulong_matrix, uint64_t, (CT_MATRIX, PT_INT64))
GET_MATRIX(get_short_matrix, int16_t, (CT_MATRIX, PT_INT16))
GET_MATRIX(get_word_matrix, uint16_t, (CT_MATRIX, PT_INT16))
GET_MATRIX(get_shortreal_matrix, float32_t, (CT_MATRIX, PT_FLOAT32))
GET_MATRIX(get_real_matrix, float64_t, (CT_MATRIX, PT_FLOAT64))
GET_MATRIX(get_longreal_matrix, floatmax_t, (CT_MATRIX, PT_FLOATMAX))
#undef GET_MATRIX

void CHDF5File::get_byte_ndarray(uint8_t*& array, int32_t*& dims, int32_t& num_dims)
{
}

void CHDF5File::get_char_ndarray(char*& array, int32_t*& dims, int32_t& num_dims)
{
}

void CHDF5File::get_int_ndarray(int32_t*& array, int32_t*& dims, int32_t& num_dims)
{
}

void CHDF5File::get_shortreal_ndarray(float32_t*& array, int32_t*& dims, int32_t& num_dims)
{
}

void CHDF5File::get_real_ndarray(float64_t*& array, int32_t*& dims, int32_t& num_dims)
{
}

void CHDF5File::get_short_ndarray(int16_t*& array, int32_t*& dims, int32_t& num_dims)
{
}

void CHDF5File::get_word_ndarray(uint16_t*& array, int32_t*& dims, int32_t& num_dims)
{
}

#define GET_SPARSEMATRIX(fname, sg_type, datatype)										\
void CHDF5File::fname(TSparse<sg_type>*& matrix, int32_t& num_feat, int32_t& num_vec)	\
{																						\
	if (!(file))																		\
		SG_ERROR("File invalid.\n");													\
}
GET_SPARSEMATRIX(get_bool_sparsematrix, bool, DT_SPARSE_BOOL)
GET_SPARSEMATRIX(get_char_sparsematrix, char, DT_SPARSE_CHAR)
GET_SPARSEMATRIX(get_byte_sparsematrix, uint8_t, DT_SPARSE_BYTE)
GET_SPARSEMATRIX(get_int_sparsematrix, int32_t, DT_SPARSE_INT)
GET_SPARSEMATRIX(get_uint_sparsematrix, uint32_t, DT_SPARSE_UINT)
GET_SPARSEMATRIX(get_long_sparsematrix, int64_t, DT_SPARSE_LONG)
GET_SPARSEMATRIX(get_ulong_sparsematrix, uint64_t, DT_SPARSE_ULONG)
GET_SPARSEMATRIX(get_short_sparsematrix, int16_t, DT_SPARSE_SHORT)
GET_SPARSEMATRIX(get_word_sparsematrix, uint16_t, DT_SPARSE_WORD)
GET_SPARSEMATRIX(get_shortreal_sparsematrix, float32_t, DT_SPARSE_SHORTREAL)
GET_SPARSEMATRIX(get_real_sparsematrix, float64_t, DT_SPARSE_REAL)
GET_SPARSEMATRIX(get_longreal_sparsematrix, floatmax_t, DT_SPARSE_LONGREAL)
#undef GET_SPARSEMATRIX


#define GET_STRING_LIST(fname, sg_type, datatype)												\
void CHDF5File::fname(T_STRING<sg_type>*& strings, int32_t& num_str, int32_t& max_string_len) \
{																								\
}

GET_STRING_LIST(get_bool_string_list, bool, DT_STRING_BOOL)
GET_STRING_LIST(get_char_string_list, char, DT_STRING_CHAR)
GET_STRING_LIST(get_byte_string_list, uint8_t, DT_STRING_BYTE)
GET_STRING_LIST(get_int_string_list, int32_t, DT_STRING_INT)
GET_STRING_LIST(get_uint_string_list, uint32_t, DT_STRING_UINT)
GET_STRING_LIST(get_long_string_list, int64_t, DT_STRING_LONG)
GET_STRING_LIST(get_ulong_string_list, uint64_t, DT_STRING_ULONG)
GET_STRING_LIST(get_short_string_list, int16_t, DT_STRING_SHORT)
GET_STRING_LIST(get_word_string_list, uint16_t, DT_STRING_WORD)
GET_STRING_LIST(get_shortreal_string_list, float32_t, DT_STRING_SHORTREAL)
GET_STRING_LIST(get_real_string_list, float64_t, DT_STRING_REAL)
GET_STRING_LIST(get_longreal_string_list, floatmax_t, DT_STRING_LONGREAL)
#undef GET_STRING_LIST

/** set functions - to pass data from shogun to the target interface */

#define SET_VECTOR(fname, sg_type, dtype, h5type)							\
void CHDF5File::fname(const sg_type* vec, int32_t len)						\
{																			\
	if (h5file<0 || !vec)													\
		SG_ERROR("File or vector invalid.\n");								\
																			\
	create_group_hierarchy();												\
																			\
	hsize_t dims=(hsize_t) len;												\
	hid_t dataspace, dataset, status;										\
	dataspace=H5Screate_simple(1, &dims, NULL); 							\
	if (dataspace<0)														\
		SG_ERROR("Could not create hdf5 dataspace\n");						\
	dataset=H5Dcreate(h5file, variable_name, h5type, dataspace, H5P_DEFAULT); \
	if (dataset<0)															\
	{																		\
		SG_ERROR("Could not create hdf5 dataset - does"						\
				" dataset '%s' already exist?\n", variable_name);			\
	}																		\
	status=H5Dwrite(dataset, h5type, H5S_ALL, H5S_ALL, H5P_DEFAULT, vec);	\
	if (status<0)															\
		SG_ERROR("Failed to write hdf5 dataset\n");							\
	H5Dclose(dataset);														\
	H5Sclose(dataspace);													\
}
SET_VECTOR(set_bool_vector, bool, DT_VECTOR_BOOL, boolean_type)
SET_VECTOR(set_byte_vector, uint8_t, DT_VECTOR_BYTE, H5T_NATIVE_UINT8)
SET_VECTOR(set_char_vector, char, DT_VECTOR_CHAR, H5T_NATIVE_CHAR)
SET_VECTOR(set_int_vector, int32_t, DT_VECTOR_INT, H5T_NATIVE_INT32)
SET_VECTOR(set_shortreal_vector, float32_t, DT_VECTOR_SHORTREAL, H5T_NATIVE_FLOAT)
SET_VECTOR(set_real_vector, float64_t, DT_VECTOR_REAL, H5T_NATIVE_DOUBLE)
SET_VECTOR(set_short_vector, int16_t, DT_VECTOR_SHORT, H5T_NATIVE_INT16)
SET_VECTOR(set_word_vector, uint16_t, DT_VECTOR_WORD, H5T_NATIVE_UINT16)
#undef SET_VECTOR

#define SET_MATRIX(fname, sg_type, dtype, h5type)								\
void CHDF5File::fname(const sg_type* matrix, int32_t num_feat, int32_t num_vec)	\
{																				\
	if (h5file<0 || !matrix)													\
		SG_ERROR("File or matrix invalid.\n");									\
																				\
	create_group_hierarchy();													\
																				\
	hsize_t dims[2]={(hsize_t) num_feat, (hsize_t) num_vec};					\
	hid_t dataspace, dataset, status;											\
	dataspace=H5Screate_simple(2, dims, NULL); 									\
	if (dataspace<0)															\
		SG_ERROR("Could not create hdf5 dataspace\n");							\
	dataset=H5Dcreate(h5file, variable_name, h5type, dataspace, H5P_DEFAULT);	\
	if (dataset<0)																\
	{																			\
		SG_ERROR("Could not create hdf5 dataset - does"							\
				" dataset '%s' already exist?\n", variable_name);				\
	}																			\
	status=H5Dwrite(dataset, h5type, H5S_ALL, H5S_ALL, H5P_DEFAULT, matrix);	\
	if (status<0)																\
		SG_ERROR("Failed to write hdf5 dataset\n");								\
	H5Dclose(dataset);															\
	H5Sclose(dataspace);														\
}
SET_MATRIX(set_bool_matrix, bool, DT_DENSE_BOOL, boolean_type)
SET_MATRIX(set_char_matrix, char, DT_DENSE_CHAR, H5T_NATIVE_CHAR)
SET_MATRIX(set_byte_matrix, uint8_t, DT_DENSE_BYTE, H5T_NATIVE_UINT8)
SET_MATRIX(set_int_matrix, int32_t, DT_DENSE_INT, H5T_NATIVE_INT32)
SET_MATRIX(set_uint_matrix, uint32_t, DT_DENSE_UINT, H5T_NATIVE_UINT32)
SET_MATRIX(set_long_matrix, int64_t, DT_DENSE_LONG, H5T_NATIVE_INT64)
SET_MATRIX(set_ulong_matrix, uint64_t, DT_DENSE_ULONG, H5T_NATIVE_UINT64)
SET_MATRIX(set_short_matrix, int16_t, DT_DENSE_SHORT, H5T_NATIVE_INT16)
SET_MATRIX(set_word_matrix, uint16_t, DT_DENSE_WORD, H5T_NATIVE_UINT16)
SET_MATRIX(set_shortreal_matrix, float32_t, DT_DENSE_SHORTREAL, H5T_NATIVE_FLOAT)
SET_MATRIX(set_real_matrix, float64_t, DT_DENSE_REAL, H5T_NATIVE_DOUBLE)
SET_MATRIX(set_longreal_matrix, floatmax_t, DT_DENSE_LONGREAL, H5T_NATIVE_LDOUBLE)
#undef SET_MATRIX

#define SET_SPARSEMATRIX(fname, sg_type, dtype) 			\
void CHDF5File::fname(const TSparse<sg_type>* matrix, 	\
		int32_t num_feat, int32_t num_vec)					\
{															\
	if (!(file && matrix))									\
		SG_ERROR("File or matrix invalid.\n");				\
															\
}
SET_SPARSEMATRIX(set_bool_sparsematrix, bool, DT_SPARSE_BOOL)
SET_SPARSEMATRIX(set_char_sparsematrix, char, DT_SPARSE_CHAR)
SET_SPARSEMATRIX(set_byte_sparsematrix, uint8_t, DT_SPARSE_BYTE)
SET_SPARSEMATRIX(set_int_sparsematrix, int32_t, DT_SPARSE_INT)
SET_SPARSEMATRIX(set_uint_sparsematrix, uint32_t, DT_SPARSE_UINT)
SET_SPARSEMATRIX(set_long_sparsematrix, int64_t, DT_SPARSE_LONG)
SET_SPARSEMATRIX(set_ulong_sparsematrix, uint64_t, DT_SPARSE_ULONG)
SET_SPARSEMATRIX(set_short_sparsematrix, int16_t, DT_SPARSE_SHORT)
SET_SPARSEMATRIX(set_word_sparsematrix, uint16_t, DT_SPARSE_WORD)
SET_SPARSEMATRIX(set_shortreal_sparsematrix, float32_t, DT_SPARSE_SHORTREAL)
SET_SPARSEMATRIX(set_real_sparsematrix, float64_t, DT_SPARSE_REAL)
SET_SPARSEMATRIX(set_longreal_sparsematrix, floatmax_t, DT_SPARSE_LONGREAL)
#undef SET_SPARSEMATRIX

#define SET_STRING_LIST(fname, sg_type, dtype) \
void CHDF5File::fname(const T_STRING<sg_type>* strings, int32_t num_str)	\
{																						\
	if (!(file && strings))																\
		SG_ERROR("File or strings invalid.\n");											\
																						\
}
SET_STRING_LIST(set_bool_string_list, bool, DT_STRING_BOOL)
SET_STRING_LIST(set_char_string_list, char, DT_STRING_CHAR)
SET_STRING_LIST(set_byte_string_list, uint8_t, DT_STRING_BYTE)
SET_STRING_LIST(set_int_string_list, int32_t, DT_STRING_INT)
SET_STRING_LIST(set_uint_string_list, uint32_t, DT_STRING_UINT)
SET_STRING_LIST(set_long_string_list, int64_t, DT_STRING_LONG)
SET_STRING_LIST(set_ulong_string_list, uint64_t, DT_STRING_ULONG)
SET_STRING_LIST(set_short_string_list, int16_t, DT_STRING_SHORT)
SET_STRING_LIST(set_word_string_list, uint16_t, DT_STRING_WORD)
SET_STRING_LIST(set_shortreal_string_list, float32_t, DT_STRING_SHORTREAL)
SET_STRING_LIST(set_real_string_list, float64_t, DT_STRING_REAL)
SET_STRING_LIST(set_longreal_string_list, floatmax_t, DT_STRING_LONGREAL)
#undef SET_STRING_LIST

void CHDF5File::get_boolean_type()
{
	boolean_type=H5T_NATIVE_UCHAR;
	switch (sizeof(bool))
	{
		case 1:
			boolean_type = H5T_NATIVE_UCHAR;
			break;
		case 2:
			boolean_type = H5T_NATIVE_UINT16;
			break;
		case 4:
			boolean_type = H5T_NATIVE_UINT32;
			break;
		case 8:
			boolean_type = H5T_NATIVE_UINT64;
			break;
		default:
			SG_ERROR("Boolean type not supported on this platform\n");
	}
}

hid_t CHDF5File::get_compatible_type(H5T_class_t t_class,
									 const TSGDataType* datatype)
{
	switch (t_class)
	{
		case H5T_FLOAT:
		case H5T_INTEGER:
			switch (datatype->m_ptype)
			{
			case PT_BOOL: return boolean_type;
			case PT_CHAR: return H5T_NATIVE_UINT8;
			/* case PT_CHAR: return H5T_NATIVE_CHAR;  */
			case PT_INT16: return H5T_NATIVE_INT16;
			case PT_UINT16: return H5T_NATIVE_UINT16;
			case PT_INT32: return H5T_NATIVE_INT32;
			case PT_UINT32: return H5T_NATIVE_UINT32;
			case PT_INT64: return H5T_NATIVE_INT64;
			case PT_UINT64: return H5T_NATIVE_UINT64;
			case PT_FLOAT32: return H5T_NATIVE_FLOAT;
			case PT_FLOAT64: return H5T_NATIVE_DOUBLE;
			case PT_FLOATMAX: return H5T_NATIVE_LDOUBLE;
			case PT_SGSERIALIZABLE_PTR:
				SG_ERROR("Implementation error during writing "
						 "HDF5File!");
				return -1;
			}
		case H5T_STRING:
			SG_ERROR("Strings not supported");
			return -1;
		case H5T_VLEN:
			SG_ERROR("Variable length containers currently not supported");
			return -1;
		case H5T_ARRAY:
			SG_ERROR("Array containers currently not supported");
			return -1;
		default:
			SG_ERROR("Datatype mismatchn");
			return -1;
	}
}

void CHDF5File::get_dims(hid_t dataset, int32_t*& dims, int32_t& ndims, int64_t& total_elements)
{
	hid_t dataspace = H5Dget_space(dataset);
	if (dataspace<0)
		SG_ERROR("Error obtaining hdf5 dataspace\n");

	ndims = H5Sget_simple_extent_ndims(dataspace); 
	total_elements=H5Sget_simple_extent_npoints(dataspace);
	hsize_t* dims_out=new hsize_t[ndims];
	dims=new int32_t[ndims];
	H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
	for (int32_t i=0; i<ndims; i++)
		dims[i]=dims_out[i];
	delete[] dims_out;
	H5Sclose(dataspace);
}

void CHDF5File::create_group_hierarchy()
{
	char* vname=strdup(variable_name);
	int32_t vlen=strlen(vname);
	for (int32_t i=0; i<vlen; i++)
	{
		if (i!=0 && vname[i]=='/')
		{
			vname[i]='\0';
			hid_t g = H5Gopen(h5file, vname);
			if (g<0)
			{
				g=H5Gcreate(h5file, vname, 0);   
				if (g<0)
					SG_ERROR("Error creating group '%s'\n", vname);
				vname[i]='/';
			}
			H5Gclose(g);
		}
	}
	free(vname);
}
#endif //  HDF5
