#include "fonts.h"
#include "wx/sizer.h"
#include "wx/wx.h"

#define LCD_W 240
#define LCD_H 240
#define SCALE 3

#define UPD_TIME 50

#define MARG 10

static uint16_t fb[LCD_W * LCD_H] = {0};

class BasicDrawPane : public wxPanel
{

public:
	BasicDrawPane(wxFrame *parent) : wxPanel(parent)
	{
		tmr = new wxTimer(this, wxID_ANY);
		Bind(wxEVT_TIMER, &BasicDrawPane::OnTimer, this, tmr->GetId());
		tmr->Start(UPD_TIME);
		SetBackgroundStyle(wxBG_STYLE_PAINT);
	}
	~BasicDrawPane()
	{
		if(tmr)
		{
			tmr->Stop();
			tmr = NULL;
		}
	}

	void paintEvent(wxPaintEvent &evt)
	{
		wxPaintDC dc(this);
		render(dc);
	}
	void paintNow()
	{
		wxClientDC dc(this);
		render(dc);
	}

	void OnTimer(wxTimerEvent &event);
	void render(wxDC &dc);
	void draw(wxDC &dc, const uint16_t *array, int x, int y, int width, int height, int scale);

	wxTimer *tmr{NULL};

	DECLARE_EVENT_TABLE()
};

class MyApp : public wxApp
{
	bool OnInit();

	wxFrame *frame;
	BasicDrawPane *drawPane;

public:
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{

	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	frame = new wxFrame((wxFrame *)NULL, -1, wxT("UI playground"), wxPoint(50, 50), wxSize(MARG * 2 + LCD_W * SCALE + 20, MARG * 2 + LCD_H * SCALE + 55));

	drawPane = new BasicDrawPane((wxFrame *)frame);
	sizer->Add(drawPane, 1, wxEXPAND);

	frame->SetSizer(sizer);
	frame->SetAutoLayout(true);
	frame->Show();
	return true;
}

BEGIN_EVENT_TABLE(BasicDrawPane, wxPanel)
EVT_PAINT(BasicDrawPane::paintEvent)
EVT_TIMER(wxID_ANY, BasicDrawPane::OnTimer)
END_EVENT_TABLE()

void BasicDrawPane::draw(wxDC &dc, const uint16_t *array, int x, int y, int width, int height, int scale)
{
	if(scale < 1) scale = 1;
	int scaledWidth = width * scale, scaledHeight = height * scale;
	wxImage image(scaledWidth, scaledHeight);

	const int r_scale_factor = 255 / 31;
	const int g_scale_factor = 255 / 63;
	const int b_scale_factor = 255 / 31;

	for(int py = 0; py < scaledHeight; py++)
	{
		int srcY = py / scale;
		unsigned char *data = image.GetData() + (py * scaledWidth * 3);

		for(int px = 0; px < scaledWidth; px++)
		{
			int srcX = px / scale;
			uint16_t rgb565 = array[srcY * width + srcX];

			data[px * 3 + 0] = ((rgb565 >> 11) & 0x1F) * r_scale_factor; // R
			data[px * 3 + 1] = ((rgb565 >> 5) & 0x3F) * g_scale_factor;	 // G
			data[px * 3 + 2] = (rgb565 & 0x1F) * b_scale_factor;		 // B
		}
	}

	wxBitmap bitmap(image);
	dc.DrawBitmap(bitmap, x, y, true);
}

void BasicDrawPane::render(wxDC &dc)
{
	make_ui_array(fb, LCD_W, LCD_H, 0);
	draw(dc, fb, MARG, MARG, LCD_W, LCD_H, SCALE);
}

void BasicDrawPane::OnTimer(wxTimerEvent &event)
{
	make_ui_array(fb, LCD_W, LCD_H, UPD_TIME);
	Refresh();
}