#include "E57FormatUtils.h"

ImageFile CreateImageFile(const char *input, uint64_t size, ReadChecksumPolicy checksumPolicy = ChecksumAll)
{
    return ImageFile(input, size, checksumPolicy);
}

StructureNode CreateStructureNode(const Node& node)
{
    return StructureNode(node);
}

