class Render
{
public:
	struct stRenderData
	{
		int _x, _y;
		bool _state;
		std::string _data;
	};

	Render(void);
	void ToggleRender(byte id, int x, int y, bool rpc, bool incoming);
	void SetString(byte id, bool rpc, bool incoming, std::string string);
	void Draw(void);
	void ResetAll(void);
	void Print(std::string &str, D3DCOLOR color, int x, int y);

private:
	stRenderData _packet[2][CConfig::_packetsSize], _rpc[2][CConfig::_packetsSize]; // 0 = outcoming, 1 = incoming
	bool _enabled = false;
	stFontInfo *_font = nullptr;
};

extern Render *render;