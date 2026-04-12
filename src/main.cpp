#include <emscripten/bind.h>
#include <unordered_map>

#include "E57Exception.h"
#include "E57Format.h"
#include "E57SimpleData.h"

using namespace emscripten;
using namespace e57;

std::string GetVersion_1_0_URI() {
    return VERSION_1_0_URI;
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
        .property("rotation", &RigidBodyTransform::rotation)
        .property("translation", &RigidBodyTransform::translation)
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
        .constructor<>()
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
        .property("columnMinimum", &IndexBound::columnMinimum)
        .property("columnMaximum", &IndexBound::columnMaximum)
        .property("returnMinimum", &IndexBounds::returnMinimum)
        .property("returnMaximum", &IndexBounds::returnMaximum)
        .function("equals", &IndexBounds::operator==)
        .function("notEquals", &IndexBounds::operator!=);

}