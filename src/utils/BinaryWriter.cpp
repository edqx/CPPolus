#include "BinaryWriter.h"

BinaryWriter::BinaryWriter(int size, bool dynamic)
{
	_size = size;
	_bytes = (char*)malloc(size);
	_cursor = 0;
	_dynamic = dynamic;
}

bool BinaryWriter::Expand(int bytes)
{
	int new_pos = _cursor + bytes;

	if (new_pos <= _size)
		return true;

	if (!_dynamic)
		return false;

	void* new_bytes = realloc(_bytes, new_pos);

	if (!new_bytes)
		return false;

	_bytes = (char*)new_bytes;
	_size = new_pos;

	return true;
}

int BinaryWriter::size()
{
	return _size;
}

bool BinaryWriter::dynamic()
{
	return _dynamic;
}

char* BinaryWriter::data()
{
	return _bytes;
}

bool BinaryWriter::Goto(int pos)
{
	if (pos > _size)
		return false;

	_cursor = pos;
	return true;
}

bool BinaryWriter::Jump(int bytes)
{
	if (_cursor + bytes > _size)
		return false;

	_cursor += bytes;
	return true;
}

bool BinaryWriter::Write(BinaryWriter writer)
{
	if (!Expand(writer.size()))
		return false;

#if IS_BIG_ENDIAN
	memcpy((void*)&_bytes[_cursor], switchbytes(writer.data()), writer.size());
#else
	memcpy((void*)&_bytes[_cursor], writer.data(), writer.size());
#endif
	_cursor += writer.size();

	return true;
}

bool BinaryWriter::WriteString(std::string val)
{
	BinaryWriter writer(0, true);
	int length = val.length();
	if (!writer.WritePackedInt32(length))
		return false;

	if (!Expand(sizeof(length) + writer.size()))
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