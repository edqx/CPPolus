#include "BinaryWriter.h"

BinaryWriter::BinaryWriter(size_t _size, bool dynamic)
{
	original_sz = _size;
	size = _size;
	data = (unsigned char*)malloc(size);
	cursor = 0;
	written = 0;
	_dynamic = dynamic;
}

bool BinaryWriter::clear()
{
	if (!Realloc(original_sz))
		return false;

	memset(data, 0, original_sz);
	written = 0;
	cursor = 0;

	return true;
}

bool BinaryWriter::Realloc(size_t bytes)
{
	void* new_bytes = realloc(data, bytes);

	if (!new_bytes)
		return false;

	data = (unsigned char*)new_bytes;
	size = bytes;

	return true;
}

bool BinaryWriter::Expand(size_t bytes)
{
	size_t new_pos = cursor + bytes;

	if (new_pos <= size)
		return true;

	if (!_dynamic)
		return false;

	Realloc(bytes);

	return true;
}

bool BinaryWriter::Goto(size_t pos)
{
	if (pos > size)
		return false;

	cursor = pos;
	return true;
}

bool BinaryWriter::Jump(size_t bytes)
{
	if (cursor + bytes > size)
		return false;

	cursor += bytes;
	return true;
}

bool BinaryWriter::FitToSize()
{
	return Realloc(written);
}

bool BinaryWriter::Write(BinaryWriter writer)
{
	if (!Expand(writer.size))
		return false;

#if IS_BIG_ENDIAN
	memcpy((void*)&_bytes[_cursor], switchbytes(writer.data), writer.size);
#else
	memcpy((void*)&data[cursor], writer.data, writer.size);
#endif
	cursor += writer.size;
	written += writer.size;

	return true;
}

bool BinaryWriter::WriteString(std::string val)
{
	BinaryWriter writer(0, true);
	int length = val.length();
	if (!writer.WritePackedInt32(length))
		return false;

	if (!Expand(sizeof(length) + writer.size))
		return false;

	Write(writer);
	for (int i = 0; i < length; i++) Write(val[i]);

	return true;
}

int PackedSize(int val) {
	int size = 1;

	while (val >= 128)
	{
		size++;
		val >>= 7;
	}

	return size;
}

bool BinaryWriter::WritePackedInt32(int val)
{
	int size = PackedSize(val);

	if (!Expand(size))
		return false;

	do
	{
		char b = val & 0xFF;

		if (val >= 0x80)
		{
			b |= 0x80;
		}

		Write(b);

		val >>= 7;
	} while (val > 0);

	return true;
}