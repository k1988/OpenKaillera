
#include "modPlayer.h"
#include "module.h"
#include "common.h"
#include "jucePlaylist.h"
#include "jucePlayerPlayer.h"
#include <time.h>
#include "zlib.h"

namespace n02 {
	namespace playback {
#include "krec.h"

		SIMPLEWINDOW(ModPlayerPlaylist, "Recorded games", Colours::whitesmoke, DocumentWindow::allButtons, jucePlaylist, 700, 400);
		SIMPLEWINDOW(PlayerPlayer, "Playback", Colours::whitesmoke, DocumentWindow::allButtons, jucePlayerPlayer, 424, 176);

		void ModPlayerPlaylist::OnClose() {
			
		}

		void uiModChangeCallback(int index) {
			modHelper.activeTransportByIndex(index);
			ModPlayerPlaylist::window->waitNotifyAndCloseNotify();            
		}

		DynamicBuffer<16> playbackBuffer;
		static bool oldEncoderUsed;
		static char * defaultValue = 0;
		static short defaultValueLen = 0;
		static int numPlayers = 0;
		double progress;

		void uiEndGame() {
			playbackBuffer.seek(playbackBuffer.getSpaceLeft() -1);
			PlayerPlayer::window->getCurrentlyModalComponent()->exitModalState(0);
		}

		void krc1BeginPlayback (File & f)
		{
			FileInputStream * fis = f.createInputStream();

			if (defaultValue != 0)
				delete defaultValue;
			defaultValue = 0;
			defaultValueLen = 0;

			if (fis !=0) {
				krecFileHeader01 head;
				if (fis->read(&head, sizeof(head))==sizeof(head)) {

					if (head.version == KREC_VERSION1) {
						char * app = head.app;
						char * ver = "";
						if ((ver = strstr(app, ";")) == 0) {
							ver = "";
							oldEncoderUsed = true;
						} else {
							oldEncoderUsed = false;
							*ver=0;
							ver++;
						}

						if (strcmp(app, client->app.name)==0) {
							int length = 0;

							if (strcmp(ver, client->app.version) ==0 || (length = 1) == 1) {
								if (length == 1) {
									if (AlertWindow::showOkCancelBox(AlertWindow::WarningIcon, "Playback error", "Application version mismatch. Do you want to continue?", "Yes", "No")) {
										length = 0;
									}
								}

								if (length == 0) {
									//length = fis->readInt();
									fis->read(&length, sizeof(length));
									int leftLength = (int)(fis->getTotalLength() - fis->getPosition());
									LOG(length %i leftlen %i, length, leftLength);
									if (leftLength >= 3) {
										unsigned char * replayBuffer = commonAlloc<unsigned char>(length + 5);
										unsigned char * compressedBuffer = commonAlloc<unsigned char>(leftLength + 5);

										int i = fis->read(compressedBuffer, leftLength);
										LOG(Read %i, i);

										if (head.Compressed !=0) {
											uLongf len = length + 5;
											uncompress(replayBuffer, &len, compressedBuffer, leftLength);
											length = len;
										} else {
											memcpy(replayBuffer, compressedBuffer, length = common_min(leftLength, length));
										}
										commonFree<unsigned char>(compressedBuffer);
										playbackBuffer.resetEx(replayBuffer, length);

										LOG(Setting buffer to %i, length);

										ModPlayerPlaylist::window->setVisible(false);
										PlayerPlayer::createAndShow();

										//AlertWindow::showMessageBox(AlertWindow::InfoIcon, "Play File", f.getFullPathName());

										PlayerPlayer::cmponnt->postCommandMessage(reinterpret_cast<int>(new String("Loading game")));

										modHelper.startGame(head.game, head.playerNo, numPlayers = head.totalPlayers);

										PlayerPlayer::window->runModalLoop();
										PlayerPlayer::window->setVisible(false);
										PlayerPlayer::window->removeFromDesktop();
										GuiJUCEDisposeObject(PlayerPlayer::window);
										ModPlayerPlaylist::window->setVisible(true);
									} else {
										AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Playback error", "Replay file is too small");
									}
								}
							}
						} else {
							AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Playback error", "Record application name mismatch");								
						}
					} else {
						AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Playback error", "File version is not supported");
					}
				} else {
					AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Playback error", "Could not read header from file");
				}

				delete fis;
			}

		}

		static void N02CCNV initialize()
		{
		}
		static void N02CCNV terminate()
		{
		}
		static void N02CCNV activete()
		{
		}
		static int  N02CCNV step()
		{
			return 0;
		}
		static int  N02CCNV synchronizeGame(void * syncData, int len)
		{
			//printf("[s%i left]", playbackBuffer.getSpaceLeft());
			PlayerPlayer::cmponnt->postCommandMessage(reinterpret_cast<int>(new String("Game loaded")));
			if (oldEncoderUsed)
				return 0;
			else {
				char workingBuffer [1024];
				while (playbackBuffer.getSpaceLeft() > 3) {

					krecDataChunkHeader01 head;
					playbackBuffer.readBytes(&head, 3);
					//LOG(head.size %i, head.size);
					playbackBuffer.readBytes(workingBuffer, head.size);

					progress = (playbackBuffer.getFilledSize() / playbackBuffer.getTotalSize()) * 100;

					switch (head.type) {
					case RDATA_SDATA:
						playbackBuffer.seek(-1 * (head.size + sizeof(head)));
						return 0;
						/*memcpy(value, workingBuffer, head.size);
						return head.size;*/
						break;
					case RDATA_ADATA:
						client->gameplay.callbackAsyncData(workingBuffer, head.size);
						break;
					case RDATA_CHAT:
						client->gameplay.callbackChatReceived(workingBuffer, workingBuffer + (strlen(workingBuffer) + 1));
						break;
					case RDATA_DROP:
						client->gameplay.callbackPlayerDropped(workingBuffer, *((short*)(workingBuffer + (strlen(workingBuffer) + 1))));
						break;
					case RDATA_EOF:
						uiEndGame();
						modHelper.endGame();
						break;
					case RDATA_METADATA:
						PlayerPlayer::cmponnt->postCommandMessage(reinterpret_cast<int>(new String(workingBuffer)));
						break;
					case RDATA_COMMENT:
						PlayerPlayer::cmponnt->postCommandMessage(reinterpret_cast<int>(new String(workingBuffer)));						
						break;
					case RDATA_GSDATA:
						LOGS(Game sync data found);
						if (head.size > 0) {
							memcpy(syncData, workingBuffer, head.size);
						}
						PlayerPlayer::cmponnt->postCommandMessage(reinterpret_cast<int>(new String("Playing game")));	
						return head.size;
						break;
					case RDATA_DEFDATA:
						if (defaultValue != 0)
							delete defaultValue;
						defaultValue = new char[defaultValueLen = head.size];
						memcpy(defaultValue, workingBuffer, defaultValueLen);
						break;
					};
				}
				modHelper.endGame();
				return -1; // return data
			}
		}
		static void N02CCNV endGame()
		{
			uiEndGame();
			modHelper.endGame();
		}
		static void N02CCNV sendAsyncData(const void * value, const int len, const int module)
		{
			require(false);
		}
		static void N02CCNV sendSyncData(const void * value, const int len)
		{
			//require(false);
		}
		static int  N02CCNV recvSyncData(void * value, const int len)
		{
			//printf("[r%i left]", playbackBuffer.getSpaceLeft());
			char workingBuffer [1024];
			while (playbackBuffer.getSpaceLeft() >= sizeof(krecDataChunkHeader01)) {

				krecDataChunkHeader01 head;

				playbackBuffer.readBytes(&head, sizeof(head));

				if (playbackBuffer.getSpaceLeft() >= head.size) {
					playbackBuffer.readBytes(workingBuffer, head.size);

					progress = (playbackBuffer.getFilledSize() / playbackBuffer.getTotalSize());

					switch (head.type) {
					case RDATA_SDATA:
						if (head.size > 0) {
							memcpy(value, workingBuffer, head.size);
						} else if (head.size == 0 && defaultValue != 0) {
							char * values = reinterpret_cast<char*>(value);
							for (int x = 0; x < numPlayers; x++) {
								memcpy(&values[x * defaultValueLen], defaultValue, defaultValueLen);
							}
						} else if (head.size == -1) {
							LOGS(End data marker);
							playbackBuffer.seek(playbackBuffer.getSpaceLeft() -1);
							modHelper.endGame();
						}
						return head.size;
						break;
					case RDATA_ADATA:
						client->gameplay.callbackAsyncData(workingBuffer, head.size);
						break;
					case RDATA_CHAT:
						client->gameplay.callbackChatReceived(workingBuffer, workingBuffer + (strlen(workingBuffer) + 1));
						break;
					case RDATA_DROP:
						client->gameplay.callbackPlayerDropped(workingBuffer, *((short*)(workingBuffer + (strlen(workingBuffer) + 1))));
						break;
					case RDATA_EOF:
						LOGS(End marker);
						uiEndGame();
						modHelper.endGame();
						return -1;
						break;
					case RDATA_METADATA:
						PlayerPlayer::cmponnt->postCommandMessage(reinterpret_cast<int>(new String(workingBuffer)));
						break;
					case RDATA_COMMENT:
						PlayerPlayer::cmponnt->postCommandMessage(reinterpret_cast<int>(new String(workingBuffer)));						
						break;
					case RDATA_GSDATA:
						require(false);
						break;
					case RDATA_DEFDATA:
						if (defaultValue != 0)
							delete defaultValue;
						defaultValue = new char[defaultValueLen = head.size];
						memcpy(defaultValue, workingBuffer, defaultValueLen);
						break;
					};
				} else {
					LOGS("Not enough data");
					break;
				}
			}
			LOGS(End here);
			modHelper.endGame();
			return -1; // return data
		}
		static int  N02CCNV syncData(void * value, const int len)
		{
			return recvSyncData(value, len);
		}
		static int  N02CCNV syncDataCheck (int)
		{
			//require(false);
			return 0;
		}
		static void N02CCNV sendChat (const char * message)
		{
			require(false);
		}
		// <gui>
		static void N02CCNV activeteGui()
		{
			ModPlayerPlaylist::createAndShow();
			ModPlayerPlaylist::waitForClose();
                        ModPlayerPlaylist::window->setVisible(false);
                        GuiJUCEDisposeObject(ModPlayerPlaylist::window);
		}
		static int  N02CCNV getSelectedAutorunIndex()
		{
			return modHelper.modHandler->getByName("autoplay")->index;
		}
		static int  N02CCNV getSelectedAutorunDelay()
		{
			return 0;
		}
		static int  N02CCNV isRecordingEnabled()
		{
			return 0;
		}
		// </gui>

		static int N02CCNV getGameplayType() {
			return GAME_TYPE_PLAYBACK;
		}

		static void N02CCNV  reserved(int, int)
		{
		}



		void PlayerPlayer::OnClose()
		{
			uiEndGame();
		}



		void playFile(File & f)
		{
			if (f.existsAsFile()) {
				krc1BeginPlayback(f);
			}
		}


		void PlayListBoxModel::playSelected(int lastRowSelected)
		{
			if (lastRowSelected >= 0 && lastRowSelected < list.itemsCount()) {
				String s;
				s.printf(T("records%hs%s"),PATH_SEPERATOR, names[lastRowSelected].value);
				File f(s);
				playFile(f);				
			}
		}

		void PlayListBoxModel::saveSelected(int lastRowSelected)
		{
			if (lastRowSelected >= 0 && lastRowSelected < list.itemsCount()) {
				String s;
				s.printf(T("records%hs%s"),PATH_SEPERATOR, names[lastRowSelected].value);
				File f(s);

				FileChooser fc("Save record", f, "*.krec");
				if (fc.browseForFileToSave(true)) {
					f.copyFileTo(fc.getResult());
				}
			}
		}

		void PlayListBoxModel::playBrowse()
		{
			FileChooser fc("Select a file to play", File("records"), "*.krec");
			if (fc.browseForFileToOpen()) {
				File f = fc.getResult();
				playFile(f);
			}
		}

		void PlayListBoxModel::RePopulateList()
		{
			list.clearItems();
			names.clearItems();
			File records("records");
			records.createDirectory();
			DirectoryIterator di(records, false);
			while (di.next()) {
				File f = di.getFile();
				if (!f.isDirectory()) {
					FileInputStream * fi = f.createInputStream();
					if (fi != 0) {
						krecFileHeader01 head;
						if (fi->read(&head, sizeof(head))==sizeof(head)) {
							if (head.version == KREC_VERSION1) {
								// dont need crc field here so might as well use it to store size
								head.crc = (int)f.getSize() / 1024;
								list.addItem(head);
								FileName fn;
								f.getFileName().copyToBuffer(fn.value, sizeof(fn.value));
								names.addItem(fn);								
							}
						}
						delete fi;
					}
				}
			}
		}

		int  PlayListBoxModel::getNumRows ()
		{
			return list.itemsCount();
		}
		void  PlayListBoxModel::paintRowBackground (Graphics &g, int rowNumber, int width, int height, bool rowIsSelected)
		{
			if (rowIsSelected)
				g.fillAll (Colour(0xdd,0xdd,0xff));
		}
		void  PlayListBoxModel::paintCell (Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
		{
			krecFileHeader01 fi = list[rowNumber];
			switch (columnId) {
				case 1:
					g.drawText (FROMUTF8(fi.game), 2, 0, width - 4, height, Justification::centredLeft, true);
					break;
				case 2:
					g.drawText (FROMUTF8(fi.app), 2, 0, width - 4, height, Justification::centredLeft, true);
					break;
				case 3:
					{
						time_t xx = fi.time;
						tm * ecx = localtime(&xx);
						String s;
						s.printf(T("%02d:%02d-%02d/%02d"),ecx->tm_hour,ecx->tm_min, ecx->tm_mday, ecx->tm_mon+1);
						g.drawText (s, 2, 0, width - 4, height, Justification::centredLeft, true);
					}
					break;
				case 4:
					{
						String s;
						s.printf(T("%ukb"), fi.crc+1);
						g.drawText (s, 2, 0, width - 4, height, Justification::centredLeft, true);
					}
					break;
				case 5:
					g.drawText (FROMUTF8(fi.comment), 2, 0, width - 4, height, Justification::centredLeft, true);
					break;
				case 6:
					g.drawText (names.getItemPtr(rowNumber)->value, 2, 0, width - 4, height, Justification::centredLeft, true);
					break;
			};
		}
		void  PlayListBoxModel::cellClicked (int rowNumber, int columnId, const MouseEvent &e)
		{
		}

		void  PlayListBoxModel::cellDoubleClicked (int rowNumber, int columnId, const MouseEvent &e)
		{
			playSelected(rowNumber);
		}

		void  PlayListBoxModel::deleteKeyPressed (int lastRowSelected)
		{
			if (lastRowSelected >= 0 && lastRowSelected < list.itemsCount()) {
				String s;
				s.printf(T("records%hs%s"),PATH_SEPERATOR, names[lastRowSelected].value);
				File f(s);
				f.deleteFile();
				names.removeIndex(lastRowSelected);
				list.removeIndex(lastRowSelected);
				ModPlayerPlaylist::cmponnt->postCommandMessage(0);
			}
		}

		void  PlayListBoxModel::returnKeyPressed (int lastRowSelected)
		{
			playSelected(lastRowSelected);
		}

	};

	STDTRANSPORTDEFNNMS(trPlayer, playback);

	STDMODULE(modPlayer, "n02.player", MTYPE02_TRANSPORT, trPlayer, 0, 0, MOD02_STATUS_WORKING, "KRC1 Playback Module");

};

#include "transport.h"

namespace n02{

	namespace deferment {
		extern int numPlayers;
		extern n02AutorunInterface * autorun;
	};
	namespace autoplay {


		static void N02CCNV initialize(int playerNo, int numPlayersP, int delayParamP)
		{
			deferment::numPlayers = numPlayersP;
			deferment::autorun = reinterpret_cast<n02AutorunInterface*>(modHelper.getExtendedInterface(client, INTERFACE_AUTORUN));
		}

		static int  N02CCNV getGameplayType()
		{
			return RA_GT_PLAYBACK;
		}
		static void N02CCNV terminate()
		{
		}

		static void N02CCNV run(int drawFrame) {
			unsigned char buffer[256];
			TRACE(); int r = transport.recvSyncData(buffer, 0);
			if (r >= 0) {
				int inputLength = r / deferment::numPlayers;
				for (int x =0; x < deferment::numPlayers; x++) {
					TRACE(); deferment::autorun->callbackSetPlayerInput(buffer + (x * inputLength), x);
				}
				TRACE(); deferment::autorun->callbackRunFrame(drawFrame? GAME_RUNFRAME_DRAW: GAME_RUNFRAME_NODRAW, 1);
			}
			TRACE();
		}

	};

	using namespace autoplay;

	static RunAlgorithmInterface02 raAutoplay = {
        RA_TRAD,
        initialize,
        getGameplayType,
        terminate,
        run
    };

    STDMODULE(modAutoplay, "autoplay", MTYPE02_RUNALG, raAutoplay, APP_ATTRIBUTES_AUTORUN, 0, MOD02_STATUS_WORKING, "Generic playback module");

};