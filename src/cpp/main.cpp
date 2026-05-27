#include <emscripten.h>
#include <emscripten/bind.h>
#include <unordered_map>

#include "E57Exception.h"
#include "E57Format.h"
#include "E57SimpleData.h"
#include "E57SimpleDataExtensions.h"
#include "E57SimpleReader.h"
#include "image_header.hpp"
#include "e57_reader.hpp"
#include "e57_writer.hpp"

using namespace emscripten;
using namespace e57;

std::string GetVersion_1_0_URI() {
    return VERSION_1_0_URI;
}

Quaternion* getRotation(RigidBodyTransform& rbt)
{
    return &(rbt.rotation);
}

EMSCRIPTEN_BINDINGS(e57) {

    // Binding E57Format.h

    enum_<NodeType>("NodeType", enum_value_type::number)
        .value("TypeStructure", NodeType::TypeStructure)
        .value("TypeVector", NodeType::TypeVector)
        .value("TypeCompressedVector", NodeType::TypeCompressedVector)
        .value("TypeInteger", NodeType::TypeInteger)
        .value("TypeScaledInteger", NodeType::TypeScaledInteger)
        .value("TypeFloat", NodeType::TypeScaledInteger)
        .value("TypeString", NodeType::TypeScaledInteger)
        .value("TypeBlob", NodeType::TypeScaledInteger);

    enum_<FloatPrecision>("FloatPrecision", enum_value_type::number)
        .value("PrecisionSingle", FloatPrecision::PrecisionSingle)
        .value("PrecisionDouble", FloatPrecision::PrecisionDouble);

    enum_<MemoryRepresentation>("MemoryRepresentation", enum_value_type::number)
        .value("Int8", MemoryRepresentation::Int8)
        .value("UInt8", MemoryRepresentation::UInt8)
        .value("Int16", MemoryRepresentation::Int16)
        .value("UInt16", MemoryRepresentation::UInt16)
        .value("Int32", MemoryRepresentation::Int32)
        .value("UInt32", MemoryRepresentation::UInt32)
        .value("Int64", MemoryRepresentation::Int64)
        .value("Bool", MemoryRepresentation::Bool)
        .value("Real32", MemoryRepresentation::Real32)
        .value("Real64", MemoryRepresentation::Real64)
        .value("UString", MemoryRepresentation::UString);

    enum_<ChecksumPolicy>("ChecksumPolicy", enum_value_type::number)
        .value("ChecksumNone", ChecksumPolicy::ChecksumNone)
        .value("ChecksumSparse", ChecksumPolicy::ChecksumSparse)
        .value("ChecksumHalf", ChecksumPolicy::ChecksumHalf)
        .value("ChecksumAll", ChecksumPolicy::ChecksumAll);

    function("GetVersion_1_0_URI", &GetVersion_1_0_URI);

    class_<ImageFile>("ImageFile")
        .function("root", &ImageFile::root)
        .function("close", &ImageFile::close)
        .function("cancel", &ImageFile::cancel)
        .function("isOpen", &ImageFile::isOpen)
        .function("isWritable", &ImageFile::isWritable)
        .function("fileName", &ImageFile::fileName)
        .function("writerCount", &ImageFile::writerCount)
        .function("readerCount", &ImageFile::readerCount)
        .function("extensionsAdd", &ImageFile::extensionsAdd)
        .function("extensionsLookupPrefix", 
            select_overload<bool(const ustring&) const>(&ImageFile::extensionsLookupPrefix))
        .function("extensionsCount", &ImageFile::extensionsCount)
        .function("extensionsPrefix", &ImageFile::extensionsPrefix)
        .function("extensionsUri", &ImageFile::extensionsUri)
        .function("isElementNameExtended", &ImageFile::isElementNameExtended)
        .function("checkInvariant", &ImageFile::checkInvariant)
        .function("equals", &ImageFile::operator==)
        .function("notEquals", &ImageFile::operator!=);

    class_<Node>("Node")
        .function("type", &Node::type)
        .function("isRoot", &Node::isRoot)
        .function("parent", &Node::parent)
        .function("pathName", &Node::pathName)
        .function("elementName", &Node::elementName)
        .function("destImageFile", &Node::destImageFile)
        .function("isAttached", &Node::isAttached)
        .function("checkInvariant", &Node::checkInvariant)
        .function("equals", &Node::operator==)
        .function("notEquals", &Node::operator!=);

    class_<StructureNode>("StructureNode")
        .function("childCount", &StructureNode::childCount)
        .function("isDefined", &StructureNode::isDefined)
        .function("get", 
            select_overload<Node(int64_t) const>(&StructureNode::get))
        .function("get",
            select_overload<Node(const ustring&) const>(&StructureNode::get))
        .function("set", &StructureNode::set)
        .function("isRoot", &StructureNode::isRoot)
        .function("parent", &StructureNode::parent)
        .function("pathName", &StructureNode::pathName)
        .function("elementName", &StructureNode::elementName)
        .function("destImageFile", &StructureNode::destImageFile)
        .function("isAttached", &StructureNode::isAttached)
        .function("checkInvariant", &StructureNode::checkInvariant);

    class_<VectorNode>("VectorNode")
        .constructor<const ImageFile&, bool>()
        .constructor<const Node&>()
        .function("allowHeteroChildren", &VectorNode::allowHeteroChildren)
        .function("childCount", &VectorNode::childCount)
        .function("isDefined", &VectorNode::isDefined)
        .function("get", 
            select_overload<Node(int64_t) const>(&VectorNode::get))
        .function("get",
            select_overload<Node(const ustring&) const>(&VectorNode::get))
        .function("append", &VectorNode::append)
        .function("isRoot", &VectorNode::isRoot)
        .function("parent", &VectorNode::parent)
        .function("pathName", &VectorNode::pathName)
        .function("elementName", &VectorNode::elementName)
        .function("destImageFile", &VectorNode::destImageFile)
        .function("isAttached", &VectorNode::isAttached)
        .function("checkInvariant", &VectorNode::checkInvariant);

    class_<SourceDestBuffer>("SourceDestBuffer")
        .function("pathName", &SourceDestBuffer::pathName)
        .function("capacity", &SourceDestBuffer::capacity)
        .function("doConversion", &SourceDestBuffer::doConversion)
        .function("doScaling", &SourceDestBuffer::doScaling)
        .function("stride", &SourceDestBuffer::stride)
        .function("checkInvariant", &SourceDestBuffer::checkInvariant);

    class_<CompressedVectorReader>("CompressedVectorReader")
        .function("read", 
            select_overload<unsigned()>(&CompressedVectorReader::read))
        .function("seek", &CompressedVectorReader::seek)
        .function("close", &CompressedVectorReader::close)
        .function("isOpen", &CompressedVectorReader::isOpen)
        .function("compressedVectorNode", &CompressedVectorReader::compressedVectorNode)
        .function("checkInvariant", &CompressedVectorReader::checkInvariant);

    class_<CompressedVectorWriter>("CompressedVectorWriter")
        .function("write",
            select_overload<void(size_t)>(&CompressedVectorWriter::write))
        .function("close", &CompressedVectorWriter::close)
        .function("isOpen", &CompressedVectorWriter::isOpen)
        .function("compressedVectorNode", &CompressedVectorWriter::compressedVectorNode)
        .function("checkInvariant", &CompressedVectorWriter::checkInvariant);

    class_<CompressedVectorNode>("CompressedVectorNode")
        .function("childCount", &CompressedVectorNode::childCount)
        .function("prototype", &CompressedVectorNode::prototype)
        .function("codecs", &CompressedVectorNode::codecs)
        .function("reader", &CompressedVectorNode::reader)
        .function("isRoot", &CompressedVectorNode::isRoot)
        .function("parent", &CompressedVectorNode::parent)
        .function("pathName", &CompressedVectorNode::pathName)
        .function("elementName", &CompressedVectorNode::elementName)
        .function("destImageFile", &CompressedVectorNode::destImageFile)
        .function("isAttached", &CompressedVectorNode::isAttached)
        .function("checkInvariant", &CompressedVectorNode::checkInvariant);

    class_<IntegerNode>("IntegerNode")
        .constructor<const ImageFile&, int64_t, int64_t, int64_t>()
        .constructor<const Node&>()
        .function("value", &IntegerNode::value)
        .function("minimum", &IntegerNode::minimum)
        .function("maximum", &IntegerNode::maximum)
        .function("isRoot", &IntegerNode::isRoot)
        .function("parent", &IntegerNode::parent)
        .function("pathName", &IntegerNode::pathName)
        .function("elementName", &IntegerNode::elementName)
        .function("destImageFile", &IntegerNode::destImageFile)
        .function("isAttached", &IntegerNode::isAttached)
        .function("checkInvariant", &IntegerNode::checkInvariant);

    class_<ScaledIntegerNode>("ScaledIntegerNode")
        .function("rawValue", &ScaledIntegerNode::rawValue)
        .function("scaledValue", &ScaledIntegerNode::scaledValue)
        .function("minimum", &ScaledIntegerNode::minimum)
        .function("scaledMinimum", &ScaledIntegerNode::scaledMinimum)
        .function("maximum", &ScaledIntegerNode::maximum)
        .function("scaledMaximum", &ScaledIntegerNode::scaledMaximum)
        .function("scale", &ScaledIntegerNode::scale)
        .function("offset", &ScaledIntegerNode::offset)
        .function("isRoot", &ScaledIntegerNode::isRoot)
        .function("parent", &ScaledIntegerNode::parent)
        .function("pathName", &ScaledIntegerNode::pathName)
        .function("elementName", &ScaledIntegerNode::elementName)
        .function("destImageFile", &ScaledIntegerNode::destImageFile)
        .function("isAttached", &ScaledIntegerNode::isAttached)
        .function("checkInvariant", &ScaledIntegerNode::checkInvariant);

    class_<FloatNode>("FloatNode")
        .function("value", &FloatNode::value)
        .function("precision", &FloatNode::precision)
        .function("minimum", &FloatNode::minimum)
        .function("maximum", &FloatNode::maximum)
        .function("isRoot", &FloatNode::isRoot)
        .function("parent", &FloatNode::parent)
        .function("pathName", &FloatNode::pathName)
        .function("elementName", &FloatNode::elementName)
        .function("destImageFile", &FloatNode::destImageFile)
        .function("isAttched", &FloatNode::isAttached)
        .function("checkInvariant", &FloatNode::checkInvariant);

    class_<StringNode>("StringNode")
        .function("value", &StringNode::value)
        .function("isRoot", &StringNode::isRoot)
        .function("parent", &StringNode::parent)
        .function("pathName", &StringNode::pathName)
        .function("elementName", &StringNode::elementName)
        .function("destImageFile", &FloatNode::destImageFile)
        .function("isAttched", &FloatNode::isAttached)
        .function("checkInvariant", &FloatNode::checkInvariant);

    class_<BlobNode>("BlobNode")
        .function("byteCount", &BlobNode::byteCount)
        .function("read", &BlobNode::read, allow_raw_pointers())
        .function("write", &BlobNode::write, allow_raw_pointers())
        .function("isRoot", &StringNode::isRoot)
        .function("parent", &StringNode::parent)
        .function("pathName", &StringNode::pathName)
        .function("elementName", &StringNode::elementName)
        .function("destImageFile", &FloatNode::destImageFile)
        .function("isAttched", &FloatNode::isAttached)
        .function("checkInvariant", &FloatNode::checkInvariant);

    // Binding E57Exception.h
    
    enum_<ErrorCode>("ErrorCode", enum_value_type::number)
        .value("Success", ErrorCode::Success)
        .value("ErrorBadCVHeader", ErrorCode::ErrorBadCVHeader)
        .value("ErrorBadCVPacket", ErrorCode::ErrorBadCVPacket)
        .value("ErrorChildIndexOutOfBounds", ErrorCode::ErrorChildIndexOutOfBounds)
        .value("ErrorSetTwice", ErrorCode::ErrorSetTwice)
        .value("ErrorHomogeneousViolation", ErrorCode::ErrorHomogeneousViolation)
        .value("ErrorValueNotRepresentable", ErrorCode::ErrorValueNotRepresentable)
        .value("ErrorScaledValueNotRepresentable", ErrorCode::ErrorScaledValueNotRepresentable)
        .value("ErrorReal64TooLarge", ErrorCode::ErrorReal64TooLarge)
        .value("ErrorExpectingNumeric", ErrorCode::ErrorExpectingNumeric)
        .value("ErrorExpectingUString", ErrorCode::ErrorExpectingUString)
        .value("ErrorInternal", ErrorCode::ErrorInternal)
        .value("ErrorBadXMLFormat", ErrorCode::ErrorBadXMLFormat)
        .value("ErrorXMLParser", ErrorCode::ErrorXMLParser)
        .value("ErrorBadAPIArgument", ErrorCode::ErrorBadAPIArgument)
        .value("ErrorFileReadOnly", ErrorCode::ErrorFileReadOnly)
        .value("ErrorBadChecksum", ErrorCode::ErrorBadChecksum)
        .value("ErrorOpenFailed", ErrorCode::ErrorOpenFailed)
        .value("ErrorCloseFailed", ErrorCode::ErrorCloseFailed)
        .value("ErrorReadFailed", ErrorCode::ErrorReadFailed)
        .value("ErrorWriteFailed", ErrorCode::ErrorWriteFailed)
        .value("ErrorSeekFailed", ErrorCode::ErrorSeekFailed)
        .value("ErrorPathUndefined", ErrorCode::ErrorPathUndefined)
        .value("ErrorBadBuffer", ErrorCode::ErrorBadBuffer)
        .value("ErrorNoBufferForElement", ErrorCode::ErrorNoBufferForElement)
        .value("ErrorBufferSizeMismatch", ErrorCode::ErrorBufferSizeMismatch)
        .value("ErrorBufferDuplicatePathName", ErrorCode::ErrorBufferDuplicatePathName)
        .value("ErrorBadFileSignature", ErrorCode::ErrorBadFileSignature)
        .value("ErrorUnknownFileVersion", ErrorCode::ErrorUnknownFileVersion)
        .value("ErrorBadFileLength", ErrorCode::ErrorBadFileLength)
        .value("ErrorXMLParserInit", ErrorCode::ErrorXMLParserInit)
        .value("ErrorDuplicateNamespacePrefix", ErrorCode::ErrorDuplicateNamespacePrefix)
        .value("ErrorDuplicateNamespaceURI", ErrorCode::ErrorDuplicateNamespaceURI)
        .value("ErrorBadPrototype", ErrorCode::ErrorBadPrototype)
        .value("ErrorBadCodecs", ErrorCode::ErrorBadCodecs)
        .value("ErrorValueOutOfBounds", ErrorCode::ErrorValueOutOfBounds)
        .value("ErrorConversionRequired", ErrorCode::ErrorConversionRequired)
        .value("ErrorBadPathName", ErrorCode::ErrorBadPathName)
        .value("ErrorNotImplemented", ErrorCode::ErrorNotImplemented)
        .value("ErrorBadNodeDowncast", ErrorCode::ErrorBadNodeDowncast)
        .value("ErrorWriterNotOpen", ErrorCode::ErrorWriterNotOpen)
        .value("ErrorReaderNotOpen", ErrorCode::ErrorReaderNotOpen)
        .value("ErrorNodeUnattached", ErrorCode::ErrorNodeUnattached)
        .value("ErrorAlreadyHasParent", ErrorCode::ErrorAlreadyHasParent)
        .value("ErrorDifferentDestImageFile", ErrorCode::ErrorDifferentDestImageFile)
        .value("ErrorImageFileNotOpen", ErrorCode::ErrorImageFileNotOpen)
        .value("ErrorBuffersNotCompatible", ErrorCode::ErrorBuffersNotCompatible)
        .value("ErrorTooManyWriters", ErrorCode::ErrorTooManyWriters)
        .value("ErrorTooManyReaders", ErrorCode::ErrorTooManyReaders)
        .value("ErrorBadConfiguration", ErrorCode::ErrorBadConfiguration)
        .value("ErrorInvarianceViolation", ErrorCode::ErrorInvarianceViolation)
        .value("ErrorInvalidNodeType", ErrorCode::ErrorInvalidNodeType)
        .value("ErrorInvalidData3DValue", ErrorCode::ErrorInvalidData3DValue)
        .value("ErrorData3DReadInvalidZeroRecords", ErrorCode::ErrorData3DReadInvalidZeroRecords)
        .value("ErrorPathNameEmpty", ErrorCode::ErrorPathNameEmpty)
        .value("ErrorPathNameMalformed", ErrorCode::ErrorPathNameMalformed)
        .value("ErrorPathNameExtensionNotRegistered", ErrorCode::ErrorPathNameExtensionNotRegistered)
        .value("E57_SUCCESS", ErrorCode::E57_SUCCESS)
        .value("E57_ERROR_BAD_CV_HEADER", ErrorCode::E57_ERROR_BAD_CV_HEADER)
        .value("E57_ERROR_BAD_CV_PACKET", ErrorCode::E57_ERROR_BAD_CV_PACKET)
        .value("E57_ERROR_CHILD_INDEX_OUT_OF_BOUNDS", ErrorCode::E57_ERROR_CHILD_INDEX_OUT_OF_BOUNDS)
        .value("E57_ERROR_SET_TWICE", ErrorCode::E57_ERROR_SET_TWICE)
        .value("E57_ERROR_HOMOGENEOUS_VIOLATION", ErrorCode::E57_ERROR_HOMOGENEOUS_VIOLATION)
        .value("E57_ERROR_VALUE_NOT_REPRESENTABLE", ErrorCode::E57_ERROR_VALUE_NOT_REPRESENTABLE)
        .value("E57_ERROR_SCALED_VALUE_NOT_REPRESENTABLE", ErrorCode::E57_ERROR_SCALED_VALUE_NOT_REPRESENTABLE)
        .value("E57_ERROR_REAL64_TOO_LARGE", ErrorCode::E57_ERROR_REAL64_TOO_LARGE)
        .value("E57_ERROR_EXPECTING_NUMERIC", ErrorCode::E57_ERROR_EXPECTING_NUMERIC)
        .value("E57_ERROR_EXPECTING_USTRING", ErrorCode::E57_ERROR_EXPECTING_USTRING)
        .value("E57_ERROR_INTERNAL", ErrorCode::E57_ERROR_INTERNAL)
        .value("E57_ERROR_BAD_XML_FORMAT", ErrorCode::E57_ERROR_BAD_XML_FORMAT)
        .value("E57_ERROR_XML_PARSER", ErrorCode::E57_ERROR_XML_PARSER)
        .value("E57_ERROR_BAD_API_ARGUMENT", ErrorCode::E57_ERROR_BAD_API_ARGUMENT)
        .value("E57_ERROR_FILE_IS_READ_ONLY", ErrorCode::E57_ERROR_FILE_IS_READ_ONLY)
        .value("E57_ERROR_BAD_CHECKSUM", ErrorCode::E57_ERROR_BAD_CHECKSUM)
        .value("E57_ERROR_OPEN_FAILED", ErrorCode::E57_ERROR_OPEN_FAILED)
        .value("E57_ERROR_CLOSE_FAILED", ErrorCode::E57_ERROR_CLOSE_FAILED)
        .value("E57_ERROR_READ_FAILED", ErrorCode::E57_ERROR_READ_FAILED)
        .value("E57_ERROR_WRITE_FAILED", ErrorCode::E57_ERROR_WRITE_FAILED)
        .value("E57_ERROR_LSEEK_FAILED", ErrorCode::E57_ERROR_LSEEK_FAILED)
        .value("E57_ERROR_PATH_UNDEFINED", ErrorCode::E57_ERROR_PATH_UNDEFINED)
        .value("E57_ERROR_BAD_BUFFER", ErrorCode::E57_ERROR_BAD_BUFFER)
        .value("E57_ERROR_NO_BUFFER_FOR_ELEMENT", ErrorCode::E57_ERROR_NO_BUFFER_FOR_ELEMENT)
        .value("E57_ERROR_BUFFER_SIZE_MISMATCH", ErrorCode::E57_ERROR_BUFFER_SIZE_MISMATCH)
        .value("E57_ERROR_BUFFER_DUPLICATE_PATHNAME", ErrorCode::E57_ERROR_BUFFER_DUPLICATE_PATHNAME)
        .value("E57_ERROR_BAD_FILE_SIGNATURE", ErrorCode::E57_ERROR_BAD_FILE_SIGNATURE)
        .value("E57_ERROR_UNKNOWN_FILE_VERSION", ErrorCode::E57_ERROR_UNKNOWN_FILE_VERSION)
        .value("E57_ERROR_BAD_FILE_LENGTH", ErrorCode::E57_ERROR_BAD_FILE_LENGTH)
        .value("E57_ERROR_XML_PARSER_INIT", ErrorCode::E57_ERROR_XML_PARSER_INIT)
        .value("E57_ERROR_DUPLICATE_NAMESPACE_PREFIX", ErrorCode::E57_ERROR_DUPLICATE_NAMESPACE_PREFIX)
        .value("E57_ERROR_DUPLICATE_NAMESPACE_URI", ErrorCode::E57_ERROR_DUPLICATE_NAMESPACE_URI)
        .value("E57_ERROR_BAD_PROTOTYPE", ErrorCode::E57_ERROR_BAD_PROTOTYPE)
        .value("E57_ERROR_BAD_CODECS", ErrorCode::E57_ERROR_BAD_CODECS)
        .value("E57_ERROR_VALUE_OUT_OF_BOUNDS", ErrorCode::E57_ERROR_VALUE_OUT_OF_BOUNDS)
        .value("E57_ERROR_CONVERSION_REQUIRED", ErrorCode::E57_ERROR_CONVERSION_REQUIRED)
        .value("E57_ERROR_BAD_PATH_NAME", ErrorCode::E57_ERROR_BAD_PATH_NAME)
        .value("E57_ERROR_NOT_IMPLEMENTED", ErrorCode::E57_ERROR_NOT_IMPLEMENTED)
        .value("E57_ERROR_BAD_NODE_DOWNCAST", ErrorCode::E57_ERROR_BAD_NODE_DOWNCAST)
        .value("E57_ERROR_WRITER_NOT_OPEN", ErrorCode::E57_ERROR_WRITER_NOT_OPEN)
        .value("E57_ERROR_READER_NOT_OPEN", ErrorCode::E57_ERROR_READER_NOT_OPEN)
        .value("E57_ERROR_NODE_UNATTACHED", ErrorCode::E57_ERROR_NODE_UNATTACHED)
        .value("E57_ERROR_ALREADY_HAS_PARENT", ErrorCode::E57_ERROR_ALREADY_HAS_PARENT)
        .value("E57_ERROR_DIFFERENT_DEST_IMAGEFILE", ErrorCode::E57_ERROR_DIFFERENT_DEST_IMAGEFILE)
        .value("E57_ERROR_IMAGEFILE_NOT_OPEN", ErrorCode::E57_ERROR_IMAGEFILE_NOT_OPEN)
        .value("E57_ERROR_BUFFERS_NOT_COMPATIBLE", ErrorCode::E57_ERROR_BUFFERS_NOT_COMPATIBLE)
        .value("E57_ERROR_TOO_MANY_WRITERS", ErrorCode::E57_ERROR_TOO_MANY_WRITERS)
        .value("E57_ERROR_TOO_MANY_READERS", ErrorCode::E57_ERROR_TOO_MANY_READERS)
        .value("E57_ERROR_BAD_CONFIGURATION", ErrorCode::E57_ERROR_BAD_CONFIGURATION)
        .value("E57_ERROR_INVARIANCE_VIOLATION", ErrorCode::E57_ERROR_INVARIANCE_VIOLATION);

    function("errorCodeToString", &Utilities::errorCodeToString);

    // Binding E57SimpleData.h
    class_<Translation>("Translation")
        .property("x", &Translation::x)
        .property("y", &Translation::y)
        .property("z", &Translation::z)
        .function("equals", &Translation::operator==)
        .function("notEquals", &Translation::operator!=)
        .class_function("identity", &Translation::identity);

    class_<Quaternion>("Quaternion")
        .property("w", &Quaternion::w)
        .property("x", &Quaternion::x)
        .property("y", &Quaternion::y)
        .property("z", &Quaternion::z)
        .function("equals", &Quaternion::operator==)
        .function("notEquals", &Quaternion::operator!=)
        .class_function("identity", &Quaternion::identity);

    class_<RigidBodyTransform>("RigidBodyTransform")
        .property("rotation", &RigidBodyTransform::rotation, return_value_policy::reference())
        .property("translation", &RigidBodyTransform::translation, return_value_policy::reference())
        .function("equals", &RigidBodyTransform::operator==)
        .function("notEquals", &RigidBodyTransform::operator!=)
        .class_function("identity", &RigidBodyTransform::identity);

    class_<CartesianBounds>("CartesianBounds")
        .property("xMinimum", &CartesianBounds::xMinimum)
        .property("xMaximum", &CartesianBounds::xMaximum)
        .property("yMinimum", &CartesianBounds::yMinimum)
        .property("yMaximum", &CartesianBounds::yMaximum)
        .property("zMinimum", &CartesianBounds::zMinimum)
        .property("zMaximum", &CartesianBounds::zMaximum)
        .function("equals", &CartesianBounds::operator==)
        .function("notEquals", &CartesianBounds::operator!=);

    class_<SphericalBounds>("SphericalBounds")
        .property("rangeMinimum", &SphericalBounds::rangeMinimum)
        .property("rangeMaximum", &SphericalBounds::rangeMaximum)
        .property("elevationMinimum", &SphericalBounds::elevationMinimum)
        .property("elevationMaximum", &SphericalBounds::elevationMaximum)
        .property("azimuthStart", &SphericalBounds::azimuthStart)
        .property("azimuthEnd", &SphericalBounds::azimuthEnd)
        .function("equals", &SphericalBounds::operator==)
        .function("notEquals", &SphericalBounds::operator!=);

    class_<IndexBounds>("IndexBounds")
        .property("rowMinimum", &IndexBounds::rowMinimum)
        .property("rowMaximum", &IndexBounds::rowMaximum)
        .property("columnMinimum", &IndexBounds::columnMinimum)
        .property("columnMaximum", &IndexBounds::columnMaximum)
        .property("returnMinimum", &IndexBounds::returnMinimum)
        .property("returnMaximum", &IndexBounds::returnMaximum)
        .function("equals", &IndexBounds::operator==)
        .function("notEquals", &IndexBounds::operator!=);

    class_<IntensityLimits>("IntensityLimits")
        .property("intensityMinimum", &IntensityLimits::intensityMinimum)
        .property("intensityMaximum", &IntensityLimits::intensityMaximum)
        .function("equals", &IntensityLimits::operator==)
        .function("notEquals", &IntensityLimits::operator!=);

    class_<ColorLimits>("ColorLimits")
        .property("colorRedMinimum", &ColorLimits::colorRedMinimum)
        .property("colorRedMaximum", &ColorLimits::colorRedMaximum)
        .property("colorGreenMinimum", &ColorLimits::colorGreenMinimum)
        .property("colorGreenMaximum", &ColorLimits::colorGreenMaximum)
        .property("colorBlueMinimum", &ColorLimits::colorBlueMinimum)
        .property("colorBlueMaximum", &ColorLimits::colorBlueMaximum)
        .function("equals", &ColorLimits::operator==)
        .function("notEquals", &ColorLimits::operator!=);

    class_<DateTime>("DateTime")
        .property("dateTimeValue", &DateTime::dateTimeValue)
        .property("isAtomicClockReferenced", &DateTime::isAtomicClockReferenced)
        .function("equals", &DateTime::operator==)
        .function("notEquals", &DateTime::operator!=);

    value_object<E57Root>("E57Root")
        .field("formatName", &E57Root::formatName)
        .field("guid", &E57Root::guid)
        .field("versionMajor", &E57Root::versionMajor)
        .field("versionMinor", &E57Root::versionMinor)
        .field("e57LibraryVersion", &E57Root::e57LibraryVersion)
        .field("creationDateTime", &E57Root::creationDateTime)
        .field("data3DSize", &E57Root::data3DSize)
        .field("images2DSize", &E57Root::images2DSize)
        .field("coordinateMetadata", &E57Root::coordinateMetadata);
    
    class_<LineGroupRecord>("LineGroupRecord")
        .property("idElementValue", &LineGroupRecord::idElementValue)
        .property("startPointIndex", &LineGroupRecord::startPointIndex)
        .property("pointCount", &LineGroupRecord::pointCount)
        .property("cartesianBounds", &LineGroupRecord::cartesianBounds, return_value_policy::reference())
        .property("sphericalBounds", &LineGroupRecord::sphericalBounds, return_value_policy::reference());

    value_object<GroupingByLine>("GroupingByLine")
        .field("idElementName", &GroupingByLine::idElementName)
        .field("groupsSize", &GroupingByLine::groupsSize)
        .field("pointCountSize", &GroupingByLine::pointCountSize);

    class_<PointGroupingSchemes>("PointGroupingSchemes")
        .property("groupingByLine", &PointGroupingSchemes::groupingByLine, return_value_policy::reference());

    enum_<NumericalNodeType>("NumericalNodeType", enum_value_type::number)
        .value("Integer", NumericalNodeType::Integer)
        .value("ScaledInteger", NumericalNodeType::ScaledInteger)
        .value("Float", NumericalNodeType::Float)
        .value("Double", NumericalNodeType::Double);

    class_<PointStandardizedFieldsAvailable>("PointStandardizedFieldsAvailable")
        .property("cartesianXField", &PointStandardizedFieldsAvailable::cartesianXField)
        .property("cartesianYField", &PointStandardizedFieldsAvailable::cartesianYField)
        .property("cartesianZField", &PointStandardizedFieldsAvailable::cartesianZField)
        .property("cartesianInvalidStateField", &PointStandardizedFieldsAvailable::cartesianInvalidStateField)
        .property("sphericalRangeField", &PointStandardizedFieldsAvailable::sphericalRangeField)
        .property("sphericalAzimuthField", &PointStandardizedFieldsAvailable::sphericalAzimuthField)
        .property("sphericalElevationField", &PointStandardizedFieldsAvailable::sphericalElevationField)
        .property("sphericalInvalidStateField", &PointStandardizedFieldsAvailable::sphericalInvalidStateField)
        .property("pointRangeMinimum", &PointStandardizedFieldsAvailable::pointRangeMinimum)
        .property("pointRangeMaximum", &PointStandardizedFieldsAvailable::pointRangeMaximum)
        .property("pointRangeNodeType", &PointStandardizedFieldsAvailable::pointRangeNodeType)
        .property("pointRangeScale", &PointStandardizedFieldsAvailable::pointRangeScale)
        .property("angleMinimum", &PointStandardizedFieldsAvailable::angleMinimum)
        .property("angleMaximum", &PointStandardizedFieldsAvailable::angleMaximum)
        .property("angleNodeType", &PointStandardizedFieldsAvailable::angleNodeType)
        .property("angleScale", &PointStandardizedFieldsAvailable::angleScale)
        .property("rowIndexField", &PointStandardizedFieldsAvailable::rowIndexField)
        .property("rowIndexMaximum", &PointStandardizedFieldsAvailable::rowIndexMaximum)
        .property("columnIndexField", &PointStandardizedFieldsAvailable::columnIndexField)
        .property("columnIndexMaximum", &PointStandardizedFieldsAvailable::columnIndexMaximum)
        .property("returnIndexField", &PointStandardizedFieldsAvailable::returnIndexField)
        .property("returnCountField", &PointStandardizedFieldsAvailable::returnCountField)
        .property("returnMaximum", &PointStandardizedFieldsAvailable::returnMaximum)
        .property("timeStampField", &PointStandardizedFieldsAvailable::timeStampField)
        .property("isTimeStampInvalidField", &PointStandardizedFieldsAvailable::isTimeStampInvalidField)
        .property("timeMinimum", &PointStandardizedFieldsAvailable::timeMinimum)
        .property("timeMaximum", &PointStandardizedFieldsAvailable::timeMaximum)
        .property("timeNodeType", &PointStandardizedFieldsAvailable::timeNodeType)
        .property("timeScale", &PointStandardizedFieldsAvailable::timeScale)
        .property("intensityField", &PointStandardizedFieldsAvailable::intensityField)
        .property("isIntensityInvalidField", &PointStandardizedFieldsAvailable::isIntensityInvalidField)
        .property("intensityNodeType", &PointStandardizedFieldsAvailable::intensityNodeType)
        .property("intensityScale", &PointStandardizedFieldsAvailable::intensityScale)
        .property("colorRedField", &PointStandardizedFieldsAvailable::colorRedField)
        .property("colorGreenField", &PointStandardizedFieldsAvailable::colorGreenField)
        .property("colorBlueField", &PointStandardizedFieldsAvailable::colorBlueField)
        .property("isColorInvalidField", &PointStandardizedFieldsAvailable::isColorInvalidField)
        .property("normalXField", &PointStandardizedFieldsAvailable::normalXField)
        .property("normalYField", &PointStandardizedFieldsAvailable::normalYField)
        .property("normalZField", &PointStandardizedFieldsAvailable::normalZField);

    class_<Data3D>("Data3D")
        .constructor<>()
        .property("name", &Data3D::name)
        .property("guid", &Data3D::guid)
        .property("originalGuids", &Data3D::originalGuids)
        .property("description", &Data3D::description)
        .property("sensorVendor", &Data3D::sensorVendor)
        .property("sensorModel", &Data3D::sensorModel)
        .property("sensorSerialNumber", &Data3D::sensorSerialNumber)
        .property("sensorHardwareVersion", &Data3D::sensorHardwareVersion)
        .property("sensorSoftwareVersion", &Data3D::sensorSoftwareVersion)
        .property("sensorFirmwareVersion", &Data3D::sensorFirmwareVersion)
        .property("temperature", &Data3D::temperature)
        .property("relativeHumidity", &Data3D::relativeHumidity)
        .property("atmosphericPressure", &Data3D::atmosphericPressure)
        .property("acquisitionStart", &Data3D::acquisitionStart)
        .property("acquisitionEnd", &Data3D::acquisitionEnd)
        .property("pose", &Data3D::pose, return_value_policy::reference())
        .property("indexBounds", &Data3D::indexBounds, return_value_policy::reference())
        .property("cartesianBounds", &Data3D::cartesianBounds, return_value_policy::reference())
        .property("sphericalBounds", &Data3D::sphericalBounds, return_value_policy::reference())
        .property("intensityLimits", &Data3D::intensityLimits, return_value_policy::reference())
        .property("colorLimits", &Data3D::colorLimits, return_value_policy::reference())
        .property("pointGroupingSchemes", &Data3D::pointGroupingSchemes, return_value_policy::reference())
        .property("pointFields", &Data3D::pointFields, return_value_policy::reference())
        .property("pointCount", &Data3D::pointCount);

    class_<VisualReferenceRepresentation>("VisualReferenceRepresentation")
        .property("jpegImageSize", &VisualReferenceRepresentation::jpegImageSize)
        .property("pngImageSize", &VisualReferenceRepresentation::pngImageSize)
        .property("imageMaskSize", &VisualReferenceRepresentation::imageMaskSize)
        .property("imageWidth", &VisualReferenceRepresentation::imageWidth)
        .property("imageHeight", &VisualReferenceRepresentation::imageHeight)
        .function("equals", &VisualReferenceRepresentation::operator==)
        .function("notEquals", &VisualReferenceRepresentation::operator!=);
    
    class_<PinholeRepresentation>("PinholeRepresentation")
        .property("jpegImageSize", &PinholeRepresentation::jpegImageSize)
        .property("pngImageSize", &PinholeRepresentation::pngImageSize)
        .property("imageMaskSize", &PinholeRepresentation::imageMaskSize)
        .property("imageWidth", &PinholeRepresentation::imageWidth)
        .property("imageHeight", &PinholeRepresentation::imageHeight)
        .property("focalLength", &PinholeRepresentation::focalLength)
        .property("pixelWidth", &PinholeRepresentation::pixelWidth)
        .property("pixelHeight", &PinholeRepresentation::pixelHeight)
        .property("principalPointX", &PinholeRepresentation::principalPointX)
        .property("principalPointY", &PinholeRepresentation::principalPointY)
        .function("equals", &PinholeRepresentation::operator==)
        .function("notEquals", &PinholeRepresentation::operator!=);

    class_<SphericalRepresentation>("SphericalRepresentation")
        .property("jpegImageSize", &SphericalRepresentation::jpegImageSize)
        .property("pngImageSize", &SphericalRepresentation::pngImageSize)
        .property("imageMaskSize", &SphericalRepresentation::imageMaskSize)
        .property("imageWidth", &SphericalRepresentation::imageWidth)
        .property("imageHeight", &SphericalRepresentation::imageWidth)
        .property("pixelWidth", &SphericalRepresentation::pixelWidth)
        .function("equals", &SphericalRepresentation::operator==)
        .function("notEquals", &SphericalRepresentation::operator!=);

    class_<CylindricalRepresentation>("CylindricalRepresentation")
        .property("jpegImageSize", &CylindricalRepresentation::jpegImageSize)
        .property("pngImageSize", &CylindricalRepresentation::pngImageSize)
        .property("imageMaskSize", &CylindricalRepresentation::imageMaskSize)
        .property("imageWidth", &CylindricalRepresentation::imageWidth)
        .property("imageHeight", &CylindricalRepresentation::imageHeight)
        .property("pixelWidth", &CylindricalRepresentation::pixelWidth)
        .property("pixelHeight", &CylindricalRepresentation::pixelHeight)
        .property("radius", &CylindricalRepresentation::radius)
        .property("principalPointY", &CylindricalRepresentation::principalPointY)
        .function("equals", &CylindricalRepresentation::operator==)
        .function("notEquals", &CylindricalRepresentation::operator!=);
    
    enum_<Image2DType>("Image2DType", enum_value_type::number)
        .value("ImageNone", Image2DType::ImageNone)
        .value("ImageJPEG", Image2DType::ImageJPEG)
        .value("ImagePNG", Image2DType::ImagePNG)
        .value("ImageMaskPNG", Image2DType::ImageMaskPNG)
        .value("E57_NO_IMAGE", Image2DType::E57_NO_IMAGE)
        .value("E57_JPEG_IMAGE", Image2DType::E57_JPEG_IMAGE)
        .value("E57_PNG_IMAGE", Image2DType::E57_PNG_IMAGE)
        .value("E57_PNG_IMAGE_MASK", Image2DType::E57_PNG_IMAGE_MASK);

    enum_<Image2DProjection>("Image2DProjection", enum_value_type::number)
        .value("ProjectionNone", Image2DProjection::ProjectionNone)
        .value("ProjectionVisual", Image2DProjection::ProjectionVisual)
        .value("ProjectionPinhole", Image2DProjection::ProjectionPinhole)
        .value("ProjectionSpherical", Image2DProjection::ProjectionSpherical)
        .value("ProjectionCylindrical", Image2DProjection::ProjectionCylindrical)
        .value("E57_NO_PROJECTION", Image2DProjection::E57_NO_PROJECTION)
        .value("E57_VISUAL", Image2DProjection::E57_VISUAL)
        .value("E57_PINHOLE", Image2DProjection::E57_PINHOLE)
        .value("E57_SPHERICAL", Image2DProjection::E57_SPHERICAL)
        .value("E57_CYLINDRICAL", Image2DProjection::E57_CYLINDRICAL);

    value_object<Extension::PinholeCameraDistortion>("PinholeCameraDistortion")
        .field("cameraNumber", &Extension::PinholeCameraDistortion::cameraNumber)
        .field("type", &Extension::PinholeCameraDistortion::type)
        .field("CV_K1", &Extension::PinholeCameraDistortion::CV_K1)
        .field("CV_K2", &Extension::PinholeCameraDistortion::CV_K2)
        .field("CV_K3", &Extension::PinholeCameraDistortion::CV_K3)
        .field("CV_K4", &Extension::PinholeCameraDistortion::CV_K4)
        .field("CV_K5", &Extension::PinholeCameraDistortion::CV_K5)
        .field("CV_K6", &Extension::PinholeCameraDistortion::CV_K6)
        .field("CV_P1", &Extension::PinholeCameraDistortion::CV_P1)
        .field("CV_P2", &Extension::PinholeCameraDistortion::CV_P2)
        .field("CV_CX", &Extension::PinholeCameraDistortion::CV_CX)
        .field("CV_CY", &Extension::PinholeCameraDistortion::CV_CY)
        .field("CV_FX", &Extension::PinholeCameraDistortion::CV_FX)
        .field("CV_FY", &Extension::PinholeCameraDistortion::CV_FY)
        .field("CV_HEIGHT", &Extension::PinholeCameraDistortion::CV_HEIGHT)
        .field("CV_WIDTH", &Extension::PinholeCameraDistortion::CV_WIDTH);

    class_<ImageHeader>("ImageHeader")
        .constructor<>()
        .property("name", &ImageHeader::name)
        .property("guid", &ImageHeader::guid)
        .property("description", &ImageHeader::description)
        .property("acquisitionDateTime", &ImageHeader::acquisitionDateTime)
        .property("associatedData3DGuid", &ImageHeader::associatedData3DGuid)
        .property("sensorVendor", &ImageHeader::sensorVendor)
        .property("sensorModel", &ImageHeader::sensorModel)
        .property("sensorSerialNumber", &ImageHeader::sensorSerialNumber)
        .property("pose", &ImageHeader::pose, return_value_policy::reference())
        .property("visualReferenceRepresentation", &ImageHeader::visualReferenceRepresentation, return_value_policy::reference())
        .property("pinholeRepresentation", &ImageHeader::pinholeRepresentation, return_value_policy::reference())
        .property("cylindricalRepresentation", &ImageHeader::cylindricalRepresentation, return_value_policy::reference())
        .property("pinholeCameraDistortionExt", &ImageHeader::pinholeCameraDistortionExt, return_value_policy::reference())
        .property("imageType", &ImageHeader::imageType)
        .property("imageProjection", &ImageHeader::imageProjection)
        .property("width", &ImageHeader::width)
        .property("height", &ImageHeader::height)
        .property("imageMaskType", &ImageHeader::imageMaskType)
        .property("imageVisualType", &ImageHeader::imageVisualType)
        .property("imageSize", &ImageHeader::imageSize);

    class_<ReaderOptions>("ReaderOptions")
        .property("checksumPolicy", &ReaderOptions::checksumPolicy, return_value_policy::reference());

    class_<Point>("Point")
        .constructor<>()
        .property("cartesianX", &Point::cartesianX)
        .property("cartesianY", &Point::cartesianY)
        .property("cartesianZ", &Point::cartesianZ)
        .property("cartesianInvalidState", &Point::cartesianInvalidState)
        .property("intensity", &Point::intensity)
        .property("isIntensityInvalid", &Point::isIntensityInvalid)
        .property("colorRed", &Point::colorRed)
        .property("colorGreen", &Point::colorGreen)
        .property("colorBlue", &Point::colorBlue)
        .property("isColorInvalid", &Point::isColorInvalid)
        .property("sphericalRange", &Point::sphericalRange)
        .property("sphericalAzimuth", &Point::sphericalAzimuth)
        .property("sphericalElevation", &Point::sphericalElevation)
        .property("rowIndex", &Point::rowIndex)
        .property("columnIndex", &Point::columnIndex)
        .property("returnIndex", &Point::returnIndex)
        .property("timeStamp", &Point::timeStamp)
        .property("isTimeStampInvalid", &Point::isTimeStampInvalid)
        .property("normalX", &Point::normalX)
        .property("normalY", &Point::normalY)
        .property("normalZ", &Point::normalZ);

    class_<E57Reader>("E57Reader")
        .constructor<const std::string&>()
        .function("GetData3DHeader", &E57Reader::GetData3DHeader)
        .function("GetImage2DHeader", &E57Reader::GetImage2DHeader)
        .function("GetHeader", &E57Reader::GetHeader)
        .function("GetData3DCount", &E57Reader::GetData3DCount)
        .function("GetImage2DCount", &E57Reader::GetImage2DCount)
        .function("ReadScan", &E57Reader::ReadScan)
        .function("ReadImage", &E57Reader::ReadImage)
        .function("TestPromise", &E57Reader::TestPromise);

    class_<E57Writer>("E57Writer")
        .constructor<const std::string&>()
        .function("AddImage", &E57Writer::AddImage, allow_raw_pointers())
        .function("AddScan", &E57Writer::AddScan, allow_raw_pointers())
        .function("Close", &E57Writer::Close);

    register_vector<Point>("VectorPoint");
    register_vector<uint8_t>("Vectoruint_8");
}
