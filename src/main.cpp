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
#endif


using namespace EE::UI::Doc;
#pragma GCC optimize("O3")



EE::Window::Window* win = NULL;

//buttons
UIPushButton* acceptButton;
UIPushButton* stopAcceptButton;
UIPushButton* cancelButton;
UIPushButton* testButton;
UIPushButton* rescanButton;

//port selector text view
UIDropDownList* portSelector;

//status views
UITextView* rawOut;
UITextView* statusOut;

//sounds
SoundBuffer answerBuf;
SoundBuffer timeoutBuf;

Sound answer;
Sound timeout;

//game status
int statusState = 0;

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
	if (port!="")
	{
		#if EE_PLATFORM == EE_PLATFORM_LINUX
			sPort.SetBaudRate(9600);
			sPort.SetDevice(port);
			sPort.Open();
		#endif
		#if EE_PLATFORM == EE_PLATFORM_WINDOWS
		#endif
	}
}

void closeSerial()
{
	#if EE_PLATFORM == EE_PLATFORM_LINUX
		if (sPort.GetState()==mn::CppLinuxSerial::State::OPEN)
		{
			sPort.Close();
		}
	#endif
}
void sendSerial(String text)
{
	#if EE_PLATFORM == EE_PLATFORM_LINUX
		if (sPort.GetState()==mn::CppLinuxSerial::State::OPEN)
		{
			sPort.Write(text);
		}
	#endif
}

String readSerial()
{
	std::string readData;
	#if EE_PLATFORM == EE_PLATFORM_LINUX
		if (sPort.GetState()==mn::CppLinuxSerial::State::OPEN)
		{
			try
			{
				sPort.Read(readData);
			}
			catch(std::system_error){
				std::cout<<"Serial port address is bad, port disconnected?\n";
				statusState = 5;
				std::cout<<"Attempting to close port\n";
				sPort.Close();
				portSelector->getListBox()->clear();
				portSelector->getListBox()->addListBoxItems(getPorts());
			}
		}
	#endif
	return readData;
}





String statusStrings[6] = {"Waiting for initialization","Idle","Accepting answers","Answering...","Testing mode","Bad port, USB disconnected?"};
void mainLoop() {
	win->getInput()->update();
	
	
	
	String readText = "";
	//serial port reading
	#if EE_PLATFORM == EE_PLATFORM_LINUX
		if (sPort.GetState()==mn::CppLinuxSerial::State::OPEN)
		{
			readText = readSerial();
		}
	#endif
	
	if (readText.contains("idle"))
	{
		statusState = 1;
	}
	else if (readText.contains("accepting"))
	{
		statusState = 2;
	}
	else if (readText.contains("answering"))
	{
		if (statusState!=3)
		{
			answer.play();
			acceptButton->setBackgroundColor(Color::gray);
		}
		statusState = 3;
	}
	else if (readText.contains("testing"))
	{
		statusState = 4;
	}
	
	rawOut->setText("Raw output: "+readText);
	statusOut->setText("Status: "+statusStrings[statusState]);
		

	//UI updating
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
		portSelector = uiSceneNode->find<UIDropDownList>("portselector");
		portSelector->getListBox()->clear();
		portSelector->getListBox()->addListBoxItems(getPorts());
		
		portSelector->on(Event::OnTextChanged,[](const Event*) {
			openSerial(portSelector->getText());
		});
		
		//status output thingies
		rawOut = uiSceneNode->find<UITextView>("rawtitle");
		statusOut = uiSceneNode->find<UITextView>("statustitle");
		
		//button setups
		//automatic storage duration setting thing ig
		acceptButton = uiSceneNode->find<UIPushButton>("accept_answers");
		stopAcceptButton = uiSceneNode->find<UIPushButton>("stop_accepting");
		cancelButton = uiSceneNode->find<UIPushButton>("cancel_answer");
		testButton = uiSceneNode->find<UIPushButton>("testmode");
		rescanButton = uiSceneNode->find<UIPushButton>("rescan");
		
		acceptButton->onClick([](const MouseEvent*) {
			acceptButton->setBackgroundColor(Color::lime);
			sendSerial("accept");
		}, EE_BUTTON_LEFT);
		
		stopAcceptButton->onClick([](const MouseEvent*) {
			acceptButton->setBackgroundColor(Color::gray);
			testButton->setBackgroundColor(Color::gray);
			timeout.play();
			sendSerial("stop");
		}, EE_BUTTON_LEFT);
		
		cancelButton->onClick([](const MouseEvent*) {
			sendSerial("cancel");
		}, EE_BUTTON_LEFT);
		
		testButton->onClick([](const MouseEvent*) {
			testButton->setBackgroundColor(Color::lime);
			sendSerial("test");
		}, EE_BUTTON_LEFT);
		rescanButton->onClick([](const MouseEvent*) {
			portSelector->getListBox()->clear();
			portSelector->getListBox()->addListBoxItems(getPorts());
		}, EE_BUTTON_LEFT);
		
		
		
		
		answerBuf.loadFromFile("assets/sounds/answer.ogg");
		answer.setBuffer(answerBuf);
		
		timeoutBuf.loadFromFile("assets/sounds/timeout.ogg");
		timeout.setBuffer(timeoutBuf);

		
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
			closeSerial();
			//MemoryManager::showResults();
		});
		
		//main loop
		win->runMainLoop(&mainLoop);
	}
	


	Engine::destroySingleton();
	MemoryManager::showResults();

	return EXIT_SUCCESS;
}
