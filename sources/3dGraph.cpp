/**
 * 
 * @file   Ploter.h
 * @author ���c �T��
 * @brief  �f�[�^���R�����O���t�ŕ\������B
 * 
 *///---------------------------------------------------------------------------
#if 0
#define     _WIN32_WINNT    0x0400
#endif
#include    "Ploter/Ploter.h"
#include    "Resource/ResForm.h"


LRESULT  CALLBACK  mainWindowProcedure( HWND,  UINT,  WPARAM,  LPARAM );


/**
 *
 * @brief �t�@�C���_�C�A���O�̏����i�[����B
 *
 *///---------------------------------------------------------------------------
struct FileDialogInfo {
    
    const char*    mFilterList;          //  �t�B���^���X�g�B
    const char*    mDefaultExtension;    //  �����g���q�B
    const char*    mTitle;               //  �_�C�A���O�̃^�C�g���B
};


namespace {
    
    HINSTANCE    current;        //  ���݂̃v���Z�X�̃C���X�^���X�n���h���B
    int          commandShow;    //  �E�B���h�E�̏�ԁB
    
    
    const FileDialogInfo    openPlotFile  =  {
        
        "�f�[�^�t�@�C��(*.txt)\0*.txt\0*.*\0*.*\0\0",
        "txt",
        "�t�@�C�����J��"
    };
}


/**
 *
 * @brief �G���[���b�Z�[�W���`�B
 *
 *///---------------------------------------------------------------------------
class Exception {
    
  public:
    
    Exception( const std::string&  type,   const std::string&  cause )  :
        
        mMessage( "Error : "  +  type  +  "  --  "  +  cause )
    {}
    
    
    BOOL  operator ()( void ) const {
        
        MessageBox( 0,  mMessage.c_str(),  "Error",  MB_OK );
        
        return  FALSE;
    }
    
  private:
    
    std::string    mMessage;     //  �G���[���b�Z�[�W�B
};


/**
 *
 * @brief �E�B���h�E���`�B
 *
 *///---------------------------------------------------------------------------
class Window {
    
  public:
    
    Window( WNDPROC  windowProcedure,   const char*  menuName,   const char*  className )  throw( Exception )  :
        
        mMenuName ( menuName  ),
        mClassName( className )
    {
        WNDCLASSEX     windowClass;
        
        
        windowClass.cbSize         =  sizeof( WNDCLASSEX );
        windowClass.style          =  CS_HREDRAW  |  CS_VREDRAW;
        windowClass.lpfnWndProc    =  windowProcedure;
        windowClass.cbClsExtra     =  0;
        windowClass.cbWndExtra     =  0;
        windowClass.hInstance      =  ::current;
        windowClass.hbrBackground  =  static_cast< HBRUSH >( GetStockObject( WHITE_BRUSH ) );
        windowClass.lpszMenuName   =  static_cast< LPCSTR >( mMenuName                     );
        windowClass.lpszClassName  =  static_cast< LPCSTR >( mClassName                    );
        
        
        windowClass.hIcon = static_cast<HICON>(
            
            LoadImage(
                
                0,
                MAKEINTRESOURCE(IDI_APPLICATION),
                IMAGE_ICON,
                0,0,
                LR_DEFAULTSIZE | LR_SHARED
            )
        );
        windowClass.hCursor = static_cast<HCURSOR>(
            
            LoadImage(
                
                0,
                MAKEINTRESOURCE(IDC_ARROW),
                IMAGE_CURSOR,
                0,0,
                LR_DEFAULTSIZE | LR_SHARED
            )
        );
        windowClass.hIconSm = static_cast<HICON>(
            
            LoadImage(
                
                0,
                MAKEINTRESOURCE(IDI_APPLICATION),
                IMAGE_ICON,
                0,0,
                LR_DEFAULTSIZE | LR_SHARED
            )
        );
        
        
        if ( ! RegisterClassEx( &windowClass ) ) {
            
            throw  Exception( "�E�B���h�E�N���X���s���ł��B",  mClassName );
        }
    }
    
    
    void  create( const char*  title,   std::size_t  width,   std::size_t  height ) const throw( Exception ) {
        
        HWND    window(
            
            CreateWindow(
                
                mClassName,
                title,
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                width,
                height,
                0,
                0,
                ::current,
                0
            )
        );
        
        
        if ( ! window ) {
            
            throw  Exception( "�E�B���h�E�����Ɏ��s���܂����B",  mClassName );
        }
        
        
        ShowWindow  ( window,  ::commandShow );
        UpdateWindow( window                 );
    }
    
  private:
    
    const char*    mMenuName;       //  ���j���[���\�[�X�̖��O�B
    const char*    mClassName;      //  �E�B���h�E�N���X�̖��O�B
};


/**
 *
 * @brief OpenGL�p�̃����_�����O�R���e�L�X�g���`�B
 *
 *///---------------------------------------------------------------------------
class OpenGL {
    
  public:
    
    OpenGL( void ) {
        
        memset( &mPixelFormat,  0,  sizeof( PIXELFORMATDESCRIPTOR ) );
        
        
        mPixelFormat.nSize       =  sizeof( PIXELFORMATDESCRIPTOR );
        mPixelFormat.nVersion    =  1;
        mPixelFormat.dwFlags     =  PFD_DRAW_TO_WINDOW  |  PFD_SUPPORT_OPENGL  |  PFD_DOUBLEBUFFER;
        mPixelFormat.iPixelType  =  PFD_TYPE_RGBA;
        mPixelFormat.cColorBits  =  24;
        mPixelFormat.iLayerType  =  PFD_MAIN_PLANE;
    }
    
    
    HGLRC  operator ()( HDC  deviceContext ) const {
        
        SetPixelFormat(
            
            deviceContext,
            ChoosePixelFormat( deviceContext,  &mPixelFormat ),
            &mPixelFormat
        );
        
        return  wglCreateContext( deviceContext );
    }
    
  private:
    
    PIXELFORMATDESCRIPTOR    mPixelFormat;    //  �s�N�Z���t�H�[�}�b�g�̐ݒ�B
};


/**
 *
 * @brief �t�@�C���_�C�A���O���`�B
 *
 *///---------------------------------------------------------------------------
class FileDialogBase {
    
  public:
    
    const char*  getFilePath( void ) const {
        
        return  mFilePath;
    }
    
  protected:
    
    FileDialogBase( HWND  window,   DWORD  flags,   const FileDialogInfo&  info ) {
        
        memset( &mOpenFileName,  0,  sizeof( OPENFILENAME ) );
        
        
        mOpenFileName.lStructSize     =  sizeof( OPENFILENAME );
        mOpenFileName.hwndOwner       =  window;
        mOpenFileName.lpstrFilter     =  info.mFilterList;
        mOpenFileName.lpstrFile       =  mFilePath;
        mOpenFileName.lpstrFileTitle  =  mFileTitle;
        mOpenFileName.nMaxFile        =  MAX_PATH;
        mOpenFileName.nMaxFileTitle   =  MAX_PATH;
        mOpenFileName.hInstance       =  0;
        mOpenFileName.lpfnHook        =  0;
        mOpenFileName.Flags           =  flags;
        mOpenFileName.lpstrDefExt     =  info.mDefaultExtension;
        mOpenFileName.lpstrTitle      =  info.mTitle;
        
        
      #if( _WIN32_WINNT  >=  0x0500 )
        
        mOpenFileName.pvReserved  =  0;
        
      #endif
    }
    
    
    void  initialize( void ) {
        
        *mFilePath   =  0;
        *mFileTitle  =  0;
    }
    
    
    OPENFILENAME    mOpenFileName;      //  �t�@�C���_�C�A���O�B
    char            mFilePath [257];    //  �t�@�C���̃p�X���B
    char            mFileTitle[257];    //  �t�@�C���̃^�C�g�����B
};


/**
 *
 * @brief �I�[�v���t�@�C���_�C�A���O���`�B
 *
 *///---------------------------------------------------------------------------
class OpenFileDialog  :  public FileDialogBase {
    
  public:
    
    OpenFileDialog( HWND  window,   const FileDialogInfo&  info )  :
        
        FileDialogBase(
            
            window,
            OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
            info
        )
    {}
    
    
    bool  operator ()( void ) {
        
        initialize();
        
        return  GetOpenFileName( &mOpenFileName );
    }
};


/**
 *
 * @brief ���C���������`�B
 *
 *///---------------------------------------------------------------------------
class Main {
    
  public:
    
    Main( void )  :  mMainWindow( mainWindowProcedure,  "IDM_MENU",  "main" ) {
        
        mMainWindow.create( "OpenGL 3D Graph",  640, 480 );
    }
    
    
    int  operator ()( void ) {
        
        try                            {  sendMessage();  }
        catch( Exception&  exception ) {  exception  ();  }
        
        return  static_cast<int>( mMessage.wParam );
    }
    
  private:
    
    void  sendMessage( void ) {
        
        while ( isLoop() ) {
            
            TranslateMessage( &mMessage );
            DispatchMessage ( &mMessage );
        }
    }
    
    
    BOOL  getMessege( void ) {
        
        return  mState = GetMessage( &mMessage,  0,0,0 );
    }
    
    
    BOOL  isLoop( void ) throw( Exception ) {
        
        if ( getMessege() == -1 ) {
            
            throw  Exception( "���b�Z�[�W���s���ł�",  "-1" );
        }
        
        return  mState;
    }
    
    
    MSG       mMessage;      //  ���b�Z�[�W�B
    BOOL      mState;        //  ���b�Z�[�W�擾�̏�ԁB
    Window    mMainWindow;   //  ���C���E�B���h�E�B
};


/**
 *
 * @brief ���C���E�B���h�E�̃v���V�[�W�����`�B
 *
 *///---------------------------------------------------------------------------
class MainWindowProcedure {
    
  public:
    
    MainWindowProcedure( HWND  window )  :
        
        mWindow          ( window                     ),
        mDeviceContext   ( GetDC   ( mWindow        ) ),
        mRenderingContext( OpenGL()( mDeviceContext ) )
    {
        paint();
    }
    
    
    LRESULT  operator ()( UINT  message,   WPARAM  wParameter,   LPARAM  lParameter ) {
        
        switch ( message ) {
            
          case  WM_PAINT   : paint  ();              break;
          case  WM_SIZE    : size   ();              break;
          case  WM_DESTROY : destroy();              break;
          case  WM_COMMAND : command( wParameter );  break;
            
          default:
            
            return  DefWindowProc( mWindow,  message,  wParameter,  lParameter );
        }
        
        return  0;
    }
    
  private:
    
    void  command( WPARAM  wParameter ) {
        
        switch ( LOWORD( wParameter ) ) {
            
          case  IDM_OPEN : open();  break;
          case  IDM_EXIT : exit();  break;
            
        }
    }
    
    
    void  paint( void ) {
        
        PAINTSTRUCT  paint;    //  �`����B
        
        BeginPaint( mWindow,  &paint );  {
            
            wglMakeCurrent( mDeviceContext,  mRenderingContext );
            SwapBuffers   ( mDeviceContext                     );
            
        }  EndPaint( mWindow,  &paint );
    }
    
    
    void  size( void ) {
        
        GetClientRect ( mWindow,                 &mClientTerritory       );
        mPloter.resize( mClientTerritory.right,  mClientTerritory.bottom );
    }
    
    
    void  destroy( void ) const {
        
        wglMakeCurrent  ( 0,0                      );
        wglDeleteContext( mRenderingContext        );
        ReleaseDC       ( mWindow,  mDeviceContext );
        PostQuitMessage ( 0                        );
    }
    
    
    void  open( void ) {
        
        OpenFileDialog  openPlotFile( mWindow, ::openPlotFile );    //  �v���b�g�t�@�C�����J���B
        
        if ( openPlotFile() ) {
            
            mPloter.set( openPlotFile.getFilePath() );
            
            this->size(); SendMessage( mWindow, WM_PAINT, 0,0 );
        }
    }
    
    
    void  exit( void ) const {
        
        SendMessage( mWindow,  WM_DESTROY,  0, 0 );
    }
    
    
    HWND            mWindow;              //  �E�B���h�E�n���h���B
    HDC             mDeviceContext;       //  �f�o�C�X�R���e�L�X�g�n���h���B
    HGLRC           mRenderingContext;    //  �����_�����O�R���e�L�X�g�n���h���B
    RECT            mClientTerritory;     //  �N���C�A���g�̈�͈̔́B
    Ploter::Ploter  mPloter;              //  �`��ΏہB
};


/**
 *
 * @brief ���C���֐��B
 *
 *///---------------------------------------------------------------------------
int  WINAPI  WinMain( HINSTANCE  current,   HINSTANCE,   LPSTR,   int  commandShow ) {
    
    ::current      =  current;
    ::commandShow  =  commandShow;
    
    try                            {  return  Main()();     }
    catch( Exception&  exception ) {  return  exception();  }
}


/**
 *
 * @brief ���C���E�B���h�E�̃E�B���h�E�v���V�[�W���B
 *
 *///---------------------------------------------------------------------------
LRESULT  CALLBACK  mainWindowProcedure( HWND  window,   UINT  message,   WPARAM  wParameter,   LPARAM  lParameter ) {
    
    static  MainWindowProcedure    procedure( window );
    
    return  procedure( message,  wParameter,  lParameter );
}
