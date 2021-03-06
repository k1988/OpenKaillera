/******************************************************************************
>         .d8888b.   .d8888b.                                                 <
>        d88P  Y88b d88P  Y88b                                                <
>        888    888        888                                                <
88888b.  888    888      .d88P                                                <
888 "88b 888    888  .od888P"                                                 <
888  888 888    888 d88P"                                                     <
888  888 Y88b  d88P 888"                                                      <
888  888  "Y8888P"  888888888              Open Kaillera Arcade Netplay Library
*******************************************************************************
Copyright (c) Open Kaillera Team 2003-2008

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice must be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#include "modKaillera.h"
#include "module.h"
#include "common.h"
#include "clientgui.h"
#include "juceKailleraServerSelect.h"
#include "juceModHelpers.h"
#include "kaillera_ClientCore.h"
#include "locid.h"

namespace n02 {

    namespace kaillera {

        /************************************************************
        ** vars
        *******************************/
        char uiUsername[32];
        char uiQuitMessage[128];
        char uiLastIP[128];
        int uiConnectionSetting;
        DynamicOrderedArray<char*> uiServersIP;
        DynamicOrderedArray<char*> uiServersName;

        /************************************************************
        ** Config Table
        *******************************/
        CONFIG_START(kailleraConfig)
            CONFIG_STRVAR("nick", uiUsername, 32, "Ape")
            CONFIG_STRVAR("qmsg", uiQuitMessage, 128, "Ape Escaped!")
            CONFIG_STRVAR("ip", uiLastIP, 128, "127.0.0.1:27888")
            CONFIG_INTVAR("connection", uiConnectionSetting, 1)
            CONFIG_STRLIST("servers_ip", uiServersIP, 128)
            CONFIG_STRLIST("servers_name", uiServersName, 128)
            CONFIG_END;

        /************************************************************
        ** Misc Prototypes
        *******************************/
        // after copying IP to uiLastIP, call this to proceed for connecting
        void ConnectCallback();

        /************************************************************
        ** GUI Window
        *******************************/

        SIMPLEWINDOW(ModKailleraServerSelect, MOD_KAILLERA_FULL_NAME, Colours::whitesmoke, DocumentWindow::allButtons, juceKailleraServerSelect, 580, 330);

        // Close button press
        void ModKailleraServerSelect::OnClose(){
            ModKailleraServerSelect::cmponnt->saveConfig();
            Component::getCurrentlyModalComponent()->exitModalState(0);
        }

        // Mod changing
        void N02CCNV uiModChangeCallback(void) {
            ModKailleraServerSelect::window->closeButtonPressed();
        }


        /************************************************************
        ** GUI Callbacks
        *******************************/
        // Connect button press
        void uibtnConnectCallback() {
            ModKailleraServerSelect::window->setVisible(false);
            ConnectCallback();
            ModKailleraServerSelect::window->setVisible(true);
        }

        // Ping button press
        void uibtnPingCallback(){
			AlertWindow::showMessageBox(AlertWindow::InfoIcon, LUTF16(LID_UCON), LUTF16(LID_UCO1));
        }

        // Masters list button press
        void uibtnMastersListCallback(){
            AlertWindow::showMessageBox(AlertWindow::InfoIcon, LUTF16(LID_UCON), LUTF16(LID_UCO1));
        }

        // add server button press
        void uiAddServer(){
            if (AddNewIP(ModKailleraServerSelect::window)) {
                uiServersName.addItem(strdup(addEditGetName()));
                uiServersIP.addItem(strdup(addEditGetIP()));
            }
        }

        // edit server button press
        void uiEditServer(int index){
            if (index >= 0 && index < uiServersName.itemsCount()) {
                if (EditIP(ModKailleraServerSelect::window, uiServersName[index], uiServersIP[index])) {
                    delete uiServersName[index];
                    delete uiServersIP[index];
                    uiServersName[index] = strdup(addEditGetName());
                    uiServersIP[index] = strdup(addEditGetIP());
                    ModKailleraServerSelect::cmponnt->redrawServersRow(index);
                }
            }
        }

        // delete server button press
        void uiDeleteServer(int index){
            if (index >= 0 && index < uiServersName.itemsCount()) {
                if (AlertWindow::showOkCancelBox(AlertWindow::WarningIcon, LUTF16(LID_DEL1), LUTF16(LID_DEL2), LUTF16(LID_YES1), LUTF16(LID_NO01))) {
                    uiServersName.removeIndex(index);
                    uiServersIP.removeIndex(index);
                }
            }
        }


        /************************************************************
        ** Module interface
        *******************************/

        // Automation... Not planning to do it for kaillera module
        static void N02CCNV initialize(){}
        static void N02CCNV terminate(){}
        static void N02CCNV activete(){}
		static void N02CCNV end(){}


        static WaitableEvent * waitable = 0;

        // GUI Entry point
        static void N02CCNV activeteGui()
        {
            TRACE(); 
            if (GuiIsJuceThread()) {
                TRACE(); ConfigurationManager config(kailleraConfig);
                TRACE(); config.load("kaillera");

				TRACE(); setSwitchModeCB(uiModChangeCallback);
                TRACE(); ModKailleraServerSelect::createAndShowModal();
                TRACE(); ModKailleraServerSelect::deleteAndZeroWindow();

                TRACE(); config.save("kaillera");
            } else {
                GuiJUCEThreadCallbackLock(activeteGui);
            }
        }

		static void N02CCNV  endGui()
		{

		}

        // etc...
        static int  N02CCNV getSelectedAutorunIndex()
        {
            return autoUiSelectedMode();
        }
        static int  N02CCNV getSelectedAutorunDelay()
        {
			return autoUiSelectedModeParam();
        }
        static int  N02CCNV isRecordingEnabled()
        {
            return recorderCBGetRecordingEnabled();
        }
        static int N02CCNV getGameplayType()
        {
            return GAME_TYPE_GAMEPLAY;
        }
        static void N02CCNV  reserved(int, int){}



        /************************************************************
        ** List box model for servers list...
        *******************************/
        int  ServersListListboxModel::getNumRows()
        {
            return uiServersIP.itemsCount();
        }
        void  ServersListListboxModel::paintRowBackground (Graphics &g, int rowNumber, int width, int height, bool rowIsSelected)
        {
            if (rowIsSelected)
                g.fillAll (Colour(0xdd,0xdd,0xff));
        }
        void  ServersListListboxModel::paintCell (Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
        {
            g.setColour (Colours::black);
            if (columnId == 2) {
                const String text (FROMUTF8(uiServersIP.getItem(rowNumber)));
                g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
            } else {
                const String text (FROMUTF8(uiServersName.getItem(rowNumber)));
                g.drawText (text, 4, 0, width - 6, height, Justification::centredLeft, true);
            }
            g.setColour (Colours::black.withAlpha (0.2f));
            g.fillRect (width - 1, 0, 1, height);
        }
        void  ServersListListboxModel::cellClicked (int rowNumber, int columnId, const MouseEvent &e)
        {
            if (uiServersIP.itemsCount() > rowNumber) {
                String xxx(uiServersIP.getItem(rowNumber));
                ModKailleraServerSelect::cmponnt->updateIP(xxx);
            }
        }
        void  ServersListListboxModel::cellDoubleClicked (int rowNumber, int columnId, const MouseEvent &e)
        {
            ServersListListboxModel::returnKeyPressed(rowNumber);
        }
        void  ServersListListboxModel::deleteKeyPressed (int lastRowSelected)
        {
            uiDeleteServer(lastRowSelected);
            ModKailleraServerSelect::cmponnt->updateLV();
        }
        void  ServersListListboxModel::returnKeyPressed (int lastRowSelected)
        {
            if (lastRowSelected >= 0 && uiServersIP.itemsCount() > lastRowSelected) {
                String xxx(uiServersIP.getItem(lastRowSelected));
                ModKailleraServerSelect::cmponnt->updateIP(xxx);
                ModKailleraServerSelect::cmponnt->saveConfig();
                uibtnConnectCallback();
            }
        }
    };

    // Transport module definition
    STDTRANSPORTDEFNNMS(trModKaillera, kaillera);

    // abstraction stuff...
    STDMODULE(modKaillera, "n02.kaillera", MTYPE02_TRANSPORT, trModKaillera, 0, 0, MOD02_STATUS_WORKING, "Kaillera module"); 
};
