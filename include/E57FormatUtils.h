#pragma once

#include "E57Format.h"

using namespace e57;

// Creation of ImageFile instance
ImageFile CreateImageFile(const char *input, uint64_t size, ReadChecksumPolicy checksumPolicy = ChecksumAll);
ImageFile CreateImageFile(const ustring &fname, const ustring &mode, ReadChecksumPolicy checksumPolicy = ChecksumAll);

// Creation of StructureNode instance
StructureNode CreateStructureNode(const Node& node);
StructureNode CreateStructureNode(const ImageFile& imgFile);

// Creation of VectorNode instance
VectorNode CreateVectorNode(const ImageFile &destImageFile, bool allowHeteroChildren = false);
VectorNode CreateVectorNode(const Node &n);

// Creation of SourceDestBuffer instance
SourceDestBuffer CreateSourceDestBuffer(const ImageFile &destImageFile, const ustring &pathName, int8_t *b,
                        size_t capacity, bool doConversion = false, bool doScaling = false,
                        size_t stride = sizeof( int8_t ));
SourceDestBuffer CreateSourceDestBuffer(const ImageFile &destImageFile, const ustring &pathName, uint8_t *b,
                        size_t capacity, bool doConversion = false, bool doScaling = false,
                        size_t stride = sizeof( uint8_t ));
SourceDestBuffer CreateSourceDestBuffer(const ImageFile &destImageFile, const ustring &pathName, int16_t *b,
                        size_t capacity, bool doConversion = false, bool doScaling = false,
                        size_t stride = sizeof( int16_t ));
SourceDestBuffer CreateSourceDestBuffer(const ImageFile &destImageFile, const ustring &pathName, uint16_t *b,
                        size_t capacity, bool doConversion = false, bool doScaling = false,
                        size_t stride = sizeof( uint16_t ));
SourceDestBuffer CreateSourceDestBuffer(const ImageFile &destImageFile, const ustring &pathName, int32_t *b,
                        size_t capacity, bool doConversion = false, bool doScaling = false,
                        size_t stride = sizeof( int32_t ));
SourceDestBuffer CreateSourceDestBuffer(const ImageFile &destImageFile, const ustring &pathName, uint32_t *b,
                        size_t capacity, bool doConversion = false, bool doScaling = false,
                        size_t stride = sizeof( uint32_t ));
SourceDestBuffer CreateSourceDestBuffer(const ImageFile &destImageFile, const ustring &pathName, int64_t *b,
                        size_t capacity, bool doConversion = false, bool doScaling = false,
                        size_t stride = sizeof( int64_t ));
SourceDestBuffer CreateSourceDestBuffer(const ImageFile &destImageFile, const ustring &pathName, bool *b,
                        size_t capacity, bool doConversion = false, bool doScaling = false,
                        size_t stride = sizeof( bool ));
SourceDestBuffer CreateSourceDestBuffer(const ImageFile &destImageFile, const ustring &pathName, float *b,
                        size_t capacity, bool doConversion = false, bool doScaling = false,
                        size_t stride = sizeof( float ));
SourceDestBuffer CreateSourceDestBuffer(const ImageFile &destImageFile, const ustring &pathName, double *b,
                        size_t capacity, bool doConversion = false, bool doScaling = false,
                        size_t stride = sizeof( double ));
SourceDestBuffer CreateSourceDestBuffer(const ImageFile &destImageFile, const ustring &pathName,
                        std::vector<ustring> *b);

// Creattion of CompressedVectorNode instance


