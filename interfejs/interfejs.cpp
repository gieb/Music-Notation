
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif
#define wxUSE_LIBPNG 1
#ifndef WX_PRECOMP
    #include <wx/app.h>
    #include <wx/frame.h>
    #include <wx/menu.h>
    #include <wx/msgdlg.h>
    #include <wx/filedlg.h>
    #include <wx/image.h>
    #include <wx/dc.h>
    #include <wx/dcmemory.h>
    #include <wx/event.h>
    #include <wx/sizer.h>
    #include <wx/choicdlg.h>
    #include <wx/arrstr.h>
    #include <wx/textctrl.h>
    #include <wx/button.h>
    #include <wx/html/htmprint.h>
    #include <wx/image.h>
    #include <wx/tokenzr.h>
    #include <wx-2.8/wx/stc/stc.h>
    #include "wx/wx.h"


    #endif

#include <iostream>
#include <fstream>
#include <string>


//wxMidi headers
#include "wxMidi/wxMidi.h"
#include "../parser.tab.c"
//lomse headers
#include <lomse_doorway.h>
#include <lomse_document.h>
#include <lomse_graphic_view.h>
#include <lomse_interactor.h>
#include <lomse/lomse_presenter.h>
#include <lomse_events.h>
#include <lomse_player_gui.h>
#include <lomse_score_player.h>

using namespace lomse;

static const int brush_size = 3;



class MidiServer : public MidiServerBase
{
protected:
    wxMidiSystem*  m_pMidiSystem;
    wxMidiOutDevice*  m_pMidiOut;


    int		m_nOutDevId;
    int		m_nVoiceChannel;

public:
    MidiServer();
    ~MidiServer();


    int count_devices();


    void set_out_device(int nOutDevId);


    void test_midi_out();

    void program_change(int channel, int instr);
    void voice_change(int channel, int instr);
    void note_on(int channel, int pitch, int volume);
    void note_off(int channel, int pitch, int volume);
    void all_sounds_off();
};
class MyApp: public wxApp
{
public:
    bool OnInit();

};


class MyCanvas;


class MyFrame: public wxFrame
{
public:
    MyFrame();
    virtual ~MyFrame();


    void open_test_document();


    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

protected:

    MyCanvas* get_active_canvas() const { return m_canvas; }


    void OnOpen(wxCommandEvent& WXUNUSED(event));
    void OnZoomIn(wxCommandEvent& WXUNUSED(event));
    void OnZoomOut(wxCommandEvent& WXUNUSED(event));
    void SaveLomse(wxCommandEvent& WXUNUSED(event));
    void OnClick(wxCommandEvent& WXUNUSED(event));
    void on_midi_settings(wxCommandEvent& WXUNUSED(event));
    void on_sound_test(wxCommandEvent& WXUNUSED(event));
    void on_play_start(wxCommandEvent& WXUNUSED(event));
    void on_play_stop(wxCommandEvent& WXUNUSED(event));
    void on_play_pause(wxCommandEvent& WXUNUSED(event));

    void show_midi_settings_dlg();

    void initialize_lomse();

    void create_menu();

    LomseDoorway m_lomse;
    MyCanvas* m_canvas;
    wxTextCtrl *MainEditBox;
    wxButton *button;
    wxHtmlEasyPrinting *print;
    wxGridSizer *gs;
    wxStyledTextCtrl *style;

    MidiServer* get_midi_server();
    ScorePlayer* get_score_player();

    MidiServer* m_pMidi;
    ScorePlayer* m_pPlayer;

    DECLARE_EVENT_TABLE()
};


class MyCanvas : public wxWindow, public PlayerNoGui
{
public:
     MyCanvas(wxFrame *frame, LomseDoorway& lomse, ScorePlayer* pPlayer);
    ~MyCanvas();
     wxImage*            m_buffer;
    void update_view_content();

    //callback wrappers
    static void wrapper_update_window(void* pThis, SpEventInfo pEvent);

    //commands
    void open_test_document();
    void open_file(const wxString& fullname);
    void zoom_in();
    void zoom_out();
    void getTextFromBox(const char *text);
    void play_start();
    void play_stop();
    void play_pause();
protected:
    //event handlers
    void OnPaint(wxPaintEvent& WXUNUSED(event));
    void OnSize(wxSizeEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnMouseEvent(wxMouseEvent& event);

    void delete_rendering_buffer();
    void create_rendering_buffer(int width, int height);
    void copy_buffer_on_dc(wxDC& dc);
    void update_rendering_buffer_if_needed();
    void force_redraw();
    void update_window();
    void on_key(int x, int y, unsigned key, unsigned flags);
    unsigned get_keyboard_flags(wxKeyEvent& event);
    unsigned get_mouse_flags(wxMouseEvent& event);


    LomseDoorway&   m_lomse;
    Presenter*      m_pPresenter;


    RenderingBuffer     m_rbuf_window;

    unsigned char*      m_pdata;
    int                 m_nBufWidth, m_nBufHeight;
    ScorePlayer* m_pPlayer;


    bool    m_view_needs_redraw;


    DECLARE_EVENT_TABLE()
};



IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame;
    frame->Show(true);
    SetTopWindow(frame);

    frame->open_test_document();

    return true;
}


enum
{

    k_menu_file_open = wxID_HIGHEST + 1,
    k_menu_file_quit = wxID_EXIT,
    k_menu_help_about = wxID_ABOUT,
    k_menu_zoom_in = wxID_ZOOM_IN,
    k_menu_zoom_out = wxID_ZOOM_OUT,
    k_menu_save_lomse = wxID_SAVEAS,

    k_menu_play_start=wxID_ANY+1,
    k_menu_play_stop,
    k_menu_play_pause,
    k_menu_midi_settings,
    k_menu_midi_test,
    BUTTON_Hello = wxID_REFRESH,

};


BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(k_menu_file_quit, MyFrame::OnQuit)
    EVT_MENU(k_menu_help_about, MyFrame::OnAbout)
    EVT_MENU(k_menu_file_open, MyFrame::OnOpen)
    EVT_MENU(k_menu_zoom_in, MyFrame::OnZoomIn)
    EVT_MENU(k_menu_zoom_out, MyFrame::OnZoomOut)
    EVT_MENU(k_menu_save_lomse, MyFrame::SaveLomse)
    EVT_BUTTON( BUTTON_Hello, MyFrame::OnClick )

    EVT_MENU(k_menu_midi_settings, MyFrame::on_midi_settings)
    EVT_MENU(k_menu_midi_test, MyFrame::on_sound_test)
    EVT_MENU(k_menu_play_start, MyFrame::on_play_start)
    EVT_MENU(k_menu_play_stop, MyFrame::on_play_stop)
    EVT_MENU(k_menu_play_pause, MyFrame::on_play_pause)
END_EVENT_TABLE()


MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, _T("Music notation"),
              wxDefaultPosition, wxSize(1224,640))
        , m_pMidi(NULL)
        , m_pPlayer(NULL)
{
    create_menu();
    initialize_lomse();
    show_midi_settings_dlg();
    wxInitAllImageHandlers();
    // create our one and only child -- it will take our entire client area
    m_canvas = new MyCanvas(this, m_lomse,get_score_player());
    wxSizer *sz = new wxBoxSizer(wxVERTICAL);
    sz->Add(m_canvas, 0, wxSHAPED | wxTOP | wxBOTTOM, 2);
    gs = new wxGridSizer(2, 2, 1, 1);
    MainEditBox = new wxTextCtrl(this,  wxID_HIGHEST + 1,
      wxT("Add input cba"), wxDefaultPosition, wxDefaultSize,
      wxTE_MULTILINE | wxTE_RICH , wxDefaultValidator, wxTextCtrlNameStr);
    gs->Add(MainEditBox,0, wxEXPAND);
    gs->Add(m_canvas,0, wxEXPAND);

    button = new wxButton(this,BUTTON_Hello,_T("Generate"), wxDefaultPosition, wxDefaultSize, 0);

    sz->Add(gs, 1, wxEXPAND);
    sz->Add(button,0, wxALIGN_LEFT);
    SetSizer(sz);
}

MyFrame::~MyFrame()
{
    delete m_pMidi;
    delete m_pPlayer;
}


void MyFrame::create_menu()
{
    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(k_menu_file_open, _T("&Open..."));
    fileMenu->Append(k_menu_save_lomse, _T("&Save as..."));
    fileMenu->AppendSeparator();
    fileMenu->Append(k_menu_file_quit, _T("E&xit"));

    wxMenu* zoomMenu = new wxMenu;
    zoomMenu->Append(k_menu_zoom_in);
    zoomMenu->Append(k_menu_zoom_out);

     wxMenu *soundMenu = new wxMenu;
    soundMenu->Append(k_menu_play_start, _T("&Play"));
    soundMenu->Append(k_menu_play_stop, _T("&Stop"));
    soundMenu->Append(k_menu_play_pause, _T("Pause/&Resume"));
    soundMenu->Append(k_menu_midi_settings, _T("&Midi settings"));
    soundMenu->Append(k_menu_midi_test, _T("Midi &test"));

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, _T("&File"));
    menuBar->Append(zoomMenu, _T("&Zoom"));
    menuBar->Append(soundMenu, _T("&Sound"));





    SetMenuBar(menuBar);


}


void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true /*force to close*/);
}


void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(_T("Lomse: sample 1 for wxWidgets"),
                 _T("About wxWidgets Lomse sample"),
                 wxOK | wxICON_INFORMATION, this);
}


void MyFrame::initialize_lomse()
{



        int pixel_format = k_pix_format_rgb24;


        int resolution = 96;


        bool reverse_y_axis = false;


    m_lomse.init_library(pixel_format,resolution, reverse_y_axis);
}

//---------------------------------------------------------------------------------------
void MyFrame::open_test_document()
{
    get_active_canvas()->open_test_document();


    get_active_canvas()->SetFocus();
}

//---------------------------------------------------------------------------------------
void MyFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
    wxString defaultPath = wxT("../../../test-scores/");

    wxString filename = wxFileSelector(_("Open score"), defaultPath,
        wxEmptyString, wxEmptyString, wxT("LenMus files (*.lms *.lmd)|*.lms;*.lmd|Cba files (*.cba)|*.cba|All files (*.*)|*.*"));

    if (filename.empty())
        return;
    char sign=0;
    wxStringTokenizer tkz(filename,_T("."));
    wxString extension;
    while(tkz.HasMoreTokens()){ extension =tkz.GetNextToken(); }

    if(!extension.compare(_T("cba"))){
        ifstream myfile (filename.mb_str());
        if (myfile.is_open()){
            parse(filename.ToAscii().data());
            MainEditBox->Clear();
            std::string str((std::istreambuf_iterator<char>(myfile)), std::istreambuf_iterator<char>());
            MainEditBox->SetValue( wxString::FromUTF8(str.c_str()));
            get_active_canvas()->getTextFromBox(yylval.str->c_str());
        }

    }
    if(!extension.compare(_T("lms")) && extension.compare(_T("lmd"))) {
         get_active_canvas()->open_file(filename);
    }else{
        ifstream myfile (filename.mb_str());
        if (myfile.is_open()){
            std::string str((std::istreambuf_iterator<char>(myfile)), std::istreambuf_iterator<char>());
            MainEditBox->SetValue( wxString::FromUTF8(str.c_str()));
        }
    }
}
void MyFrame::OnClick(wxCommandEvent& WXUNUSED(event)){

    wxString mystring=MainEditBox->GetValue();
    std:ofstream out("output.txt");
    out << mystring.mb_str(wxConvUTF8);
    out.close();
    parse("output.txt");

    if(parseError == 0){
        get_active_canvas()->getTextFromBox(yylval.str->c_str());

    }
    else
    {
        wxString er(error_str.c_str(),wxConvUTF8);
        wxMessageBox(er);

    }


}
void MyFrame::SaveLomse(wxCommandEvent& WXUNUSED(event)){

    wxFileDialog saveFileDialog(this, _("Save Lomse file"),wxT(""),wxT(""),wxT("lomse files (*.lms *.lmd)|*.lms;*.lmd|Cba files (*.cba)|*.cba|Image (*.jpg)|*.jpg"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    if(saveFileDialog.GetFilterIndex() == 0){
        wxString mystring=MainEditBox->GetValue();
        std:ofstream outTmp("output.txt");
        outTmp << mystring.mb_str(wxConvUTF8);
        outTmp.close();
        parse("output.txt");

        wxString path = saveFileDialog.GetPath();

        wxStringTokenizer tkz(path,_T("."));
        wxString extension;

        while(tkz.HasMoreTokens()){ extension = tkz.GetNextToken(); }
        if(extension.compare(_T("lms")) || extension.compare(_T("lmd")))
           path.append(_T(".lms"));

        if(parseError = 1){

        }

        ofstream out(path.ToAscii().data());
        std::cout<<yylval.str;
        out << yylval.str->c_str();
        out.close();
    }if(saveFileDialog.GetFilterIndex() == 1){
        wxString path = saveFileDialog.GetPath();

        wxStringTokenizer tkz(path,_T("."));
        wxString extension;

        while(tkz.HasMoreTokens()){ extension =tkz.GetNextToken(); }
        if(extension.compare(_T("cba"))) path.append(_T(".cba"));
        wxString mystring=MainEditBox->GetValue();
        ofstream outTmp(path.ToAscii().data());
        std::cout<<mystring.mb_str(wxConvUTF8);
        outTmp << mystring.mb_str(wxConvUTF8);

        outTmp.close();
    }else{
        wxString path = saveFileDialog.GetPath();

        wxStringTokenizer tkz(path,_T("."));
        wxString extension;

        while(tkz.HasMoreTokens()){ extension =tkz.GetNextToken(); }
        if(extension.compare(_T("jpg"))) path.append(_T(".jpg"));
        get_active_canvas()->m_buffer->SaveFile(path,wxBITMAP_TYPE_JPEG);
    }


}


void MyFrame::OnZoomIn(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->zoom_in();
}


void MyFrame::OnZoomOut(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->zoom_out();
}


void MyFrame::on_play_start(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->play_start();
}

void MyFrame::on_play_stop(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->play_stop();
}

void MyFrame::on_play_pause(wxCommandEvent& WXUNUSED(event))
{
    get_active_canvas()->play_pause();
}

void MyFrame::on_midi_settings(wxCommandEvent& WXUNUSED(event))
{
    show_midi_settings_dlg();
}

void MyFrame::on_sound_test(wxCommandEvent& WXUNUSED(event))
{
    MidiServer* pMidi = get_midi_server();
    if (!pMidi) return;
    pMidi->test_midi_out();
}

void MyFrame::show_midi_settings_dlg()
{
    wxArrayString outDevices;
    vector<int> deviceIndex;

    MidiServer* pMidi = get_midi_server();
    int nNumDevices = pMidi->count_devices();
    for (int i = 0; i < nNumDevices; i++)
    {
        wxMidiOutDevice device(i);
        if (device.IsOutputPort())
        {
            outDevices.Add( device.DeviceName() );
            deviceIndex.push_back(i);
        }
    }

    int iSel = ::wxGetSingleChoiceIndex(
                            _T("Select Midi output device to use:"),
                            _T("Midi settings dlg"),
                            outDevices,
                            this
                       );
    if (iSel == -1)
    {

    }
    else
    {

        MidiServer* pMidi = get_midi_server();
        int deviceID = deviceIndex[iSel];
        pMidi->set_out_device(deviceID);
    }
}

//---------------------------------------------------------------------------------------
MidiServer* MyFrame::get_midi_server()
{
    if (!m_pMidi)
        m_pMidi = new MidiServer();
    return m_pMidi;
}

//---------------------------------------------------------------------------------------
ScorePlayer* MyFrame::get_score_player()
{
    if (!m_pPlayer)
    {
        MidiServer* pMidi = get_midi_server();
        m_pPlayer = m_lomse.create_score_player(pMidi);
    }
    return m_pPlayer;
}


BEGIN_EVENT_TABLE(MyCanvas, wxWindow)
    EVT_KEY_DOWN(MyCanvas::OnKeyDown)
    EVT_MOUSE_EVENTS(MyCanvas::OnMouseEvent)
    EVT_SIZE(MyCanvas::OnSize)
    EVT_PAINT(MyCanvas::OnPaint)
END_EVENT_TABLE()


//---------------------------------------------------------------------------------------
MyCanvas::MyCanvas(wxFrame *frame, LomseDoorway& lomse, ScorePlayer* pPlayer)
    : wxWindow(frame, wxID_ANY)
    , PlayerNoGui(60L /*tempo 60 MM*/, true /*count off*/, true /*metronome clicks*/)
    , m_lomse(lomse)
	, m_pPresenter(NULL)
	, m_buffer(NULL)
	, m_pPlayer(pPlayer)
	, m_view_needs_redraw(true)
{
}

//---------------------------------------------------------------------------------------
MyCanvas::~MyCanvas()
{
    delete_rendering_buffer();
    delete m_pPresenter;
}

//---------------------------------------------------------------------------------------
void MyCanvas::getTextFromBox(const char *text){
    try{

    std::string input(text);

    delete m_pPresenter;
    m_pPresenter = m_lomse.new_document(ViewFactory::k_view_horizontal_book,input);



    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {

        spInteractor->set_rendering_buffer(&m_rbuf_window);

        spInteractor->add_event_handler(k_update_window_event, this, wrapper_update_window);

        spInteractor->hide_caret();
    }

    m_view_needs_redraw = true;
    Refresh(false /* don't erase background */);
    }catch(...){
       wxMessageBox( wxT("Błąd składni!") );
    }
}
void MyCanvas::open_file(const wxString& fullname)

{
    //create a new View
    std::string filename( fullname.mb_str(wxConvUTF8) );
    delete m_pPresenter;

    m_pPresenter = m_lomse.open_document(ViewFactory::k_view_horizontal_book,
                                         filename);


    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {

        spInteractor->set_rendering_buffer(&m_rbuf_window);


        spInteractor->add_event_handler(k_update_window_event, this, wrapper_update_window);

        spInteractor->hide_caret();
    }


    m_view_needs_redraw = true;
    Refresh(false /* don't erase background */);
}

//---------------------------------------------------------------------------------------
void MyCanvas::OnSize(wxSizeEvent& WXUNUSED(event))
{
    wxSize size = this->GetClientSize();
    create_rendering_buffer(size.GetWidth(), size.GetHeight());

    Refresh(false /* don't erase background */);
}

//---------------------------------------------------------------------------------------
void MyCanvas::OnPaint(wxPaintEvent& event)
{
    if (!m_pPresenter)
        event.Skip(false);
    else
    {
        update_rendering_buffer_if_needed();
        wxPaintDC dc(this);
        copy_buffer_on_dc(dc);
    }
}

//---------------------------------------------------------------------------------------
void MyCanvas::update_rendering_buffer_if_needed()
{
    if (m_view_needs_redraw)
        update_view_content();

    m_view_needs_redraw = false;
}

//---------------------------------------------------------------------------------------
void MyCanvas::delete_rendering_buffer()
{
    delete m_buffer;
}

//---------------------------------------------------------------------------------------
void MyCanvas::create_rendering_buffer(int width, int height)
{

    delete_rendering_buffer();
    m_nBufWidth = width;
    m_nBufHeight = height;
    m_buffer = new wxImage(width, height);


    m_pdata = m_buffer->GetData();


    #define BYTES_PER_PIXEL 3   //wxImage  has RGB, 24 bits format
    int stride = m_nBufWidth * BYTES_PER_PIXEL;     //number of bytes per row
    m_rbuf_window.attach(m_pdata, m_nBufWidth, m_nBufHeight, stride);

    m_view_needs_redraw = true;
}

//-------------------------------------------------------------------------
void MyCanvas::open_test_document()
{




}


//---------------------------------------------------------------------------------------
void MyCanvas::force_redraw()
{
    update_view_content();
    update_window();
}

//---------------------------------------------------------------------------------------
void MyCanvas::wrapper_update_window(void* pThis, SpEventInfo pEvent)
{
    static_cast<MyCanvas*>(pThis)->update_window();
}

//---------------------------------------------------------------------------------------
void MyCanvas::update_window()
{


    wxClientDC dc(this);
    copy_buffer_on_dc(dc);
}

//---------------------------------------------------------------------------------------
void MyCanvas::copy_buffer_on_dc(wxDC& dc)
{
    if (!m_buffer || !m_buffer->IsOk())
        return;

    wxBitmap bitmap(*m_buffer);
    dc.DrawBitmap(bitmap, 0, 0, false /* don't use mask */);
}

//-------------------------------------------------------------------------
void MyCanvas::update_view_content()
{

    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
        spInteractor->redraw_bitmap();
}

//---------------------------------------------------------------------------------------
void MyCanvas::OnKeyDown(wxKeyEvent& event)
{
    if (!m_pPresenter) return;

    int nKeyCode = event.GetKeyCode();
    unsigned flags = get_keyboard_flags(event);


    if (nKeyCode > 0 && nKeyCode < 27)
    {
        nKeyCode += int('A') - 1;
        flags |= k_kbd_ctrl;
    }


    switch (nKeyCode)
    {
        case WXK_SHIFT:
        case WXK_ALT:
        case WXK_CONTROL:
            return;

        default:
            on_key(event.GetX(), event.GetY(), nKeyCode, flags);;
    }
}

//-------------------------------------------------------------------------
void MyCanvas::on_key(int x, int y, unsigned key, unsigned flags)
{
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        switch (key)
        {
            case 'D':
                spInteractor->switch_task(TaskFactory::k_task_drag_view);
                break;
            case 'S':
                spInteractor->switch_task(TaskFactory::k_task_selection);
                break;
            case '+':
                spInteractor->zoom_in(x, y);
                force_redraw();
                break;
            case '-':
                spInteractor->zoom_out(x, y);
                force_redraw();
                break;
            default:
               return;
        }
    }
}

//-------------------------------------------------------------------------
void MyCanvas::zoom_in()
{
    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        wxSize size = this->GetClientSize();
        spInteractor->zoom_in(size.GetWidth()/2, size.GetHeight()/2);
        force_redraw();
    }
}

//-------------------------------------------------------------------------
void MyCanvas::zoom_out()
{
    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        wxSize size = this->GetClientSize();
        spInteractor->zoom_out(size.GetWidth()/2, size.GetHeight()/2);
        force_redraw();
    }
}

//---------------------------------------------------------------------------------------
unsigned MyCanvas::get_mouse_flags(wxMouseEvent& event)
{
    unsigned flags = 0;
    if (event.LeftIsDown())     flags |= k_mouse_left;
    if (event.RightIsDown())    flags |= k_mouse_right;
    if (event.MiddleDown())     flags |= k_mouse_middle;
    if (event.ShiftDown())      flags |= k_kbd_shift;
    if (event.AltDown())        flags |= k_kbd_alt;
    if (event.ControlDown())    flags |= k_kbd_ctrl;
    return flags;
}


unsigned MyCanvas::get_keyboard_flags(wxKeyEvent& event)
{
    unsigned flags = 0;
    if (event.ShiftDown())   flags |= k_kbd_shift;
    if (event.AltDown()) flags |= k_kbd_alt;
    if (event.ControlDown()) flags |= k_kbd_ctrl;
    return flags;
}


void MyCanvas::OnMouseEvent(wxMouseEvent& event)
{
    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        wxEventType nEventType = event.GetEventType();
        wxPoint pos = event.GetPosition();
        unsigned flags = get_mouse_flags(event);

        if (nEventType==wxEVT_LEFT_DOWN)
        {
            flags |= k_mouse_left;
            spInteractor->on_mouse_button_down(pos.x, pos.y, flags);
        }
        else if (nEventType==wxEVT_LEFT_UP)
        {
            flags |= k_mouse_left;
            spInteractor->on_mouse_button_up(pos.x, pos.y, flags);
        }
        else if (nEventType==wxEVT_RIGHT_DOWN)
        {
            flags |= k_mouse_right;
            spInteractor->on_mouse_button_down(pos.x, pos.y, flags);
        }
        else if (nEventType==wxEVT_RIGHT_UP)
        {
            flags |= k_mouse_right;
            spInteractor->on_mouse_button_up(pos.x, pos.y, flags);
        }
        else if (nEventType==wxEVT_MOTION)
            spInteractor->on_mouse_move(pos.x, pos.y, flags);
    }
}
void MyCanvas::play_start()
{
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        Document* pDoc = m_pPresenter->get_document_raw_ptr();
        ImoScore* pScore = static_cast<ImoScore*>( pDoc->get_imodoc()->get_content_item(0) );
        if (pScore)
        {
            m_pPlayer->load_score(pScore, this);
            m_pPlayer->play(k_do_visual_tracking, 0, spInteractor.get());
        }
    }
}

void MyCanvas::play_stop()
{
    m_pPlayer->stop();
}


void MyCanvas::play_pause()
{
    m_pPlayer->pause();
}
MidiServer::MidiServer()
    : m_pMidiSystem( wxMidiSystem::GetInstance() )
    , m_pMidiOut(NULL)
    , m_nOutDevId(-1)
    , m_nVoiceChannel(0)    // 0 based. So this is channel 1
{
}


MidiServer::~MidiServer()
{
    if (m_pMidiOut)
        m_pMidiOut->Close();

    delete m_pMidiOut;
    delete m_pMidiSystem;
}


void MidiServer::set_out_device(int nOutDevId)
{
    wxMidiError nErr;


    if (!m_pMidiOut || (m_nOutDevId != nOutDevId))
    {

         if (m_pMidiOut)
         {
            nErr = m_pMidiOut->Close();
            delete m_pMidiOut;
            m_pMidiOut = NULL;
            if (nErr)
            {
                wxMessageBox( wxString::Format(
                    _T("Error %d while closing Midi device: %s \n")
                    , nErr, m_pMidiSystem->GetErrorText(nErr).c_str() ));
                return;
            }
        }


        m_nOutDevId = nOutDevId;
        if (m_nOutDevId != -1)
        {
            try
            {
                m_pMidiOut = new wxMidiOutDevice(m_nOutDevId);
                nErr = m_pMidiOut->Open(0, NULL);
            }
            catch(...)
            {
				wxLogMessage(_T("[MidiServer::set_out_device] Crash opening Midi device"));
				return;
            }

            if (nErr)
				wxMessageBox( wxString::Format(
                    _T("Error %d opening Midi device: %s \n")
                    , nErr, m_pMidiSystem->GetErrorText(nErr).c_str() ));
            else
				wxMessageBox(_T("Midi out device correctly set."));
        }
    }
}


void MidiServer::program_change(int channel, int instr)
{
    m_pMidiOut->ProgramChange(channel, instr);
}


void MidiServer::voice_change(int channel, int instrument)
{
    m_nVoiceChannel = channel;
    if (m_pMidiOut)
    {
        wxMidiError nErr = m_pMidiOut->ProgramChange(channel, instrument);
        if (nErr)
        {
            wxMessageBox( wxString::Format(
				_T("Error %d in ProgramChange:\n%s")
                , nErr, m_pMidiSystem->GetErrorText(nErr).c_str() ));
        }
    }
}


void MidiServer::note_on(int channel, int pitch, int volume)
{
    m_pMidiOut->NoteOn(channel, pitch, volume);
}


void MidiServer::note_off(int channel, int pitch, int volume)
{
    m_pMidiOut->NoteOff(channel, pitch, volume);
}


void MidiServer::all_sounds_off()
{
    m_pMidiOut->AllSoundsOff();
}


void MidiServer::test_midi_out()
{
    if (!m_pMidiOut) return;


    int scale[] = { 60, 62, 64, 65, 67, 69, 71, 72 };
    #define SCALE_SIZE 8

    for (int i = 0; i < SCALE_SIZE; i++)
    {
        m_pMidiOut->NoteOn(m_nVoiceChannel, scale[i], 100);
        ::wxMilliSleep(200);
        m_pMidiOut->NoteOff(m_nVoiceChannel, scale[i], 100);
    }
}


int MidiServer::count_devices()
{
    return m_pMidiSystem->CountDevices();
}
