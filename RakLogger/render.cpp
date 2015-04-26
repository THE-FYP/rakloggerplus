#include "main.h"

Render *render = new Render();

Render::Render(void)
{
	ResetAll();
}

void Render::ToggleRender(byte id, int x, int y, bool rpc, bool incoming)
{
	if (_font == nullptr)
		_font = SF->getRender()->CreateNewFont("Tahoma", 8, FCR_BORDER);
	int in = incoming ? 1 : 0;
	stRenderData &rd = rpc ? _rpc[in][id] : _packet[in][id];
	rd._state ^= true;
	rd._x = x, rd._y = y;
	_enabled = true;
}

void Render::SetString(byte id, bool rpc, bool incoming, std::string string)
{
	int in = incoming ? 1 : 0;
	stRenderData &rd = rpc ? _rpc[in][id] : _packet[in][id];
	rd._data = string;
}

void Render::Draw(void)
{
	if (!_enabled || _font == nullptr)
		return;
	for (size_t in = 0; in < 2; ++in)
	{
		for (size_t i = 0; i < CConfig::_packetsSize; ++i)
		{
			if (_rpc[in][i]._state && !_rpc[in][i]._data.empty())
			{
				Print(_rpc[in][i]._data, in ? COLOR_RECV : COLOR_SEND, _rpc[in][i]._x, _rpc[in][i]._y);
			}
			if (_packet[in][i]._state && !_packet[in][i]._data.empty())
			{
				Print(_packet[in][i]._data, in ? COLOR_RECV : COLOR_SEND, _packet[in][i]._x, _packet[in][i]._y);
			}
		}
	}
}

void Render::Print(std::string &str, D3DCOLOR color, int x, int y)
{
	list<string> strings;
	boost::split(strings, str, boost::is_any_of("\n\r"));
	float ny = (float) y;
	for each (auto &s in strings)
	{
		_font->Print(s.c_str(), color, (float)x, ny, false, true);
		ny += _font->DrawHeight();
	}
}

void Render::ResetAll(void)
{
	_enabled = false;
	if (_font != nullptr)
		SF->getRender()->ReleaseFont(_font), _font = nullptr;
	for (size_t in = 0; in < 2; ++in)
	{
		for (size_t i = 0; i < CConfig::_packetsSize; ++i)
		{
			_rpc[in][i]._x = _rpc[in][i]._y = _packet[in][i]._x = _packet[in][i]._y = 0;
			_rpc[in][i]._state = _packet[in][i]._state = false;
			_rpc[in][i]._data.clear();
			_packet[in][i]._data.clear();
		}
	}
}