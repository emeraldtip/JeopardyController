#include <eepp/ee.hpp>
#include <eepp/ui/doc/syntaxdefinitionmanager.hpp>
#include <eepp/ui/doc/syntaxtokenizer.hpp>
#include <eepp/ui/doc/textdocument.hpp>
#include <filesystem>
#include <iostream>
#include <future>
#include <vector>

#if EE_PLATFORM == EE_PLATFORM_LINUX
	#include <CppLinuxSerial/SerialPort.hpp>
#endif
#if EE_PLATFORM == EE_PLATFORM_WINDOWS
	#include <serial/serialib>
#endif


using namespace EE::UI::Doc;
#pragma GCC optimize("O3")



EE::Window::Window* win = NULL;

//buttons
UIWidget* acceptButton;
UIWidget* stopAccceptButton;
UIWidget* cancelButton;
UIWidget* testButton;
UIWidget* rescanButton;

//port selector text view
UIWidget* portSelector;

//sounds
SoundBuffer mismBuf;
SoundBuffer answerBuf;
SoundBuffer timeoutBuf;

Sound mism;
Sound answer;
Sound timeout;


//serial control
#if EE_PLATFORM == EE_PLATFORM_LINUX

	mn::CppLinuxSerial::SerialPort sPort("", mn::CppLinuxSerial::BaudRate::B_9600);
#endif
#if EE_PLATFORM == EE_PLATFORM_WINDOWS
	HANDLE sPort;
#endif

std::vector<String> getPorts()
{
	std::vector<String> ports;
	//linux implementation
	#if EE_PLATFORM == EE_PLATFORM_LINUX
		std::string path = "/dev";
		for (const auto & entry : std::filesystem::directory_iterator(path))
		{
			if (entry.path().u8string().find("USB") != std::string::npos)
			{	
				ports.push_back(entry.path().u8string());
			}
		}
	#endif
	
	//windows implementation
	return ports;
}

void openSerial(String port)
{
	#if EE_PLATFORM == EE_PLATFORM_LINUX
		sPort.SetBaudRate(9600);
		sPort.SetDevice(port);
		sPort.Open();
	#endif
	#if EE_PLATFORM == EE_PLATFORM_LINUX
		
	#endif
}

void closeSerial()
{
	#if EE_PLATFORM == EE_PLATFORM_LINUX
		sPort.Close();
	#endif
}

void sendSerial(String text)
{
	#if EE_PLATFORM == EE_PLATFORM_LINUX
		sPort.Write(text);
	#endif
}

String readSerial()
{
	std::string readData;
	#if EE_PLATFORM == EE_PLATFORM_LINUX
		sPort.Read(readData);
	#endif
	return readData;
}



void mainLoop() {
	win->getInput()->update();
		
/*
	if (win->getInput()->isKeyUp(KEY_ESCAPE)) {
		win->close();
	}
*/

	//update the UI scene.
	SceneManager::instance()->update();

	if (SceneManager::instance()->getUISceneNode()->invalidated()) {
		win->clear();
		SceneManager::instance()->draw();
		win->display();
	} 
	else {
		win->getInput()->waitEvent( Milliseconds(win->hasFocus() ? 16 : 100));
	}
}




EE_MAIN_FUNC int main(int, char**) {
	win = Engine::instance()->createWindow(WindowSettings(1920, 1080, "Jeopardy controller"),
											ContextSettings(true));

	if ( win->isOpen() ) {
		//change current working directory to app directory so resource path is always correct
		FileSystem::changeWorkingDirectory(Sys::getProcessPath());

		FontTrueType* font = FontTrueType::New( "NotoSans-Regular", "assets/fonts/NotoSans-Regular.ttf" );

		//scene node shenanigans
		UISceneNode* uiSceneNode = UISceneNode::New();
		uiSceneNode->getUIThemeManager()->setDefaultFont(font);
		SceneManager::instance()->add(uiSceneNode);

		std::cout << "stupid\n";	
		//layout loading
		uiSceneNode->loadLayoutFromFile("assets/layouts/layout.xml");

		//style loading
		StyleSheetParser parser;
		parser.loadFromFile("assets/styles/style.css");
		uiSceneNode->setStyleSheet(parser.getStyleSheet());
		
		//port selector 
		auto portSelector = uiSceneNode->find<UIDropDownList>("portselector");
		portSelector->getListBox()->clear();
		portSelector->getListBox()->addListBoxItems(getPorts());
		
		
		//button setups
		//automatic storage duration setting thing ig
		auto acceptButton = uiSceneNode->find<UIPushButton>("accept_answers");
		auto stopAcceptButton = uiSceneNode->find<UIPushButton>("stop_accepting");
		auto cancelButton = uiSceneNode->find<UIPushButton>("cancel_answer");
		auto testButton = uiSceneNode->find<UIPushButton>("testmode");
		auto rescanButton = uiSceneNode->find<UIPushButton>("rescan");
		
		acceptButton->onClick([acceptButton](const MouseEvent*) {
			acceptButton->setBackgroundColor(Color::lime);
			sendSerial("accept");
		}, EE_BUTTON_LEFT);
		
		stopAcceptButton->onClick([acceptButton,testButton](const MouseEvent*) {
			acceptButton->setBackgroundColor(Color::gray);
			testButton->setBackgroundColor(Color::gray);
			sendSerial("stop");
		}, EE_BUTTON_LEFT);
		
		cancelButton->onClick([](const MouseEvent*) {
			sendSerial("cancel");
		}, EE_BUTTON_LEFT);
		
		testButton->onClick([testButton](const MouseEvent*) {
			testButton->setBackgroundColor(Color::lime);
			sendSerial("test");
		}, EE_BUTTON_LEFT);
		rescanButton->onClick([portSelector](const MouseEvent*) {
			portSelector->getListBox()->clear();
			portSelector->getListBox()->addListBoxItems(getPorts());
		}, EE_BUTTON_LEFT);
		
		
		
		
		//sounds
		mismBuf.loadFromFile("assets/sounds/MIS_MÕTTES.ogg");
		mism.setBuffer(mismBuf);
		mism.play();
		answerBuf.loadFromFile("assets/sounds/answer.ogg");
		answer.setBuffer(answerBuf);
		answer.play();
		timeoutBuf.loadFromFile("assets/sounds/timeout.ogg");
		timeout.setBuffer(timeoutBuf);
		timeout.play();

		
		//widget setup stuff
		//UIWidget* widget = uiSceneNode->find<UIPushButton>("button_view");
		//widget->onClick([widget](const MouseEvent*) {widget->setBackgroundColor(Color::red);}, EE_BUTTON_RIGHT);
		/*
		UIImage* imag = uiSceneNode->find<U+678IImage>("img");
		Texture* tex = TextureFactory::instance()->loadFromFile("assets/testpic3.jpg");
		imag->setDrawable(tex);
		
		imag->setScaleType(UIScaleType::FitInside);
		//imag->setScale({0.9,0.9});
		//imag->setPosition({0,0});
		*/
		
		//UIGridLayout* vbox = uiSceneNode->find<UIGridLayout>("imglayout");
		//vbox->setSize({win->getHeight()*0.8,win->getWidth()*0.8});
		
		//UIWidget* layout = uiSceneNode->find<UILinearLayout>("layout");
		//layout->on(Event::OnSizeChange,[vbox](const Event*) {vbox->setSize({win->getHeight()*0.8f,win->getWidth()*0.8f});});
		
		//layout->addEventListener(Event::OnWindowClose, [](const Event* event) {std::cout<<"overso"<<std::endl;destroyLoad = true;});
		
		//UIWidget* openbutton = uiSceneNode->find<UIPushButton>("button_view2");
		//openbutton->onClick([openbutton,uiSceneNode](const MouseEvent*) {showFileDialog("Open folder", [uiSceneNode](const Event* event) {openFolder(event,uiSceneNode);},"*",UIFileDialog::AllowFolderSelect);});
		
		
		win->setQuitCallback([](EE::Window::Window* w){
			std::cout<<"Attempting to close open serial ports...\n";
			#if EE_PLATFORM == EE_PLATFORM_LINUX
				sPort.Close();
			#endif
			#if EE_PLATFORM == EE_PLATFORM_WINDOWS
				//Insert windows port closing code here
			#endif
			//MemoryManager::showResults();
		});
		
		//main loop
		win->runMainLoop(&mainLoop);
	}
	


	Engine::destroySingleton();
	MemoryManager::showResults();

	return EXIT_SUCCESS;
}
