/*
 * Copyright 2009, Ilio Catallo, Stefano Celentano, Eleonora Ciceri, all rights reserved
 * Distribuited under the terms of the GPL v2 license
 * 
 * Authors:
 *
 *			Ilio Catallo
 *			Eleonora Ciceri
 * 
 * Last revision: Ilio catallo, 6th June 2009
 *
 * Description: TODO
 */
 
#include "NoteApplication.h"

#include <Alert.h>
#include <Autolock.h>
#include <Entry.h>
#include <File.h>
#include <Mime.h>
#include <Node.h>
#include <Path.h>

#define COLOR_CHANGED 	'ccrq'
#define FONT_BOLD 		'fntb'

NoteApplication *note_app;


// This function fills the image_info image struct with the app image of itself
// and eventually return B_OK
status_t our_image(image_info& image){
	
	int32 cookie = 0;
	
	while (get_next_image_info(B_CURRENT_TEAM, &cookie, &image) == B_OK) {
	
		//printf("our_image %s\n",(char *)our_image);
	
		if ((char *)our_image >= (char *)image.text
			&& (char *)our_image <= (char *)image.text + image.text_size)
			return B_OK;
	}

	return B_ERROR;
}




NoteApplication :: NoteApplication()
			    : BApplication("application/x-vnd.ccc-TakeNotes"){	
	
		//Check (and install) the MIME type
		CheckMime();
	
		//private data members initialization
		fWindowCount = 0;
		fWindowCountUntitled = 0;
		note_app = this;

}	

void NoteApplication :: ReadyToRun(){

		if (fWindowCount > 0)
			return;
		else
			OpenNote();

}


void NoteApplication :: CheckMime(){

		//Variables
		BMimeType 	takenotes("application/takenotes");
		image_info 	info;
		entry_ref	ref;
		
		//	takenotes.SetTo("application/takenotes");
		
		if (takenotes.InitCheck() == B_OK){
		
			//Variables
			BMessage *msg = new BMessage();
			BMessage *attr = new BMessage();
		
			//Check if the mimetype is already installed in the DB
			if (takenotes.IsInstalled()){
			
					printf("già insatllato\n");
					
					delete msg;
					delete attr;
					
					return;
			}	
			
			//We define the extensions for the mime type
			msg->AddString("extensions","tkn");
			
			if (takenotes.SetFileExtensions(msg) != B_OK){
				printf("errore nel settare l'estensione");
				exit(-1);
			}
			
			//Set the preferred application	
			if (takenotes.SetPreferredApp("application/x-vnd.ccc-TakeNotes") != B_OK){
				printf("errore nel settare l'app preferred\n");
				exit(-1);
			}
			
			//Set the Hint application, using the our_image function we are able to find
			//the current path of the application and pass it to the FileType DB
			if (our_image(info) == B_OK && get_ref_for_path(info.name, &ref) == B_OK){
			
					BPath prova(&ref);
					printf("path %s\n",prova.Path());
					
					if (takenotes.SetAppHint(&ref) != B_OK)
						printf("errore nell'app hint\n");
			
			}
			
			//Add some extra attributes in order to manage custom tags
			attr->AddString("attr:name","TAKENOTES:refapp");
			attr->AddString("attr:name","TAKENOTES:type");
			attr->AddString("attr:name","TAKENOTES:tagone");
			attr->AddString("attr:name","TAKENOTES:tagtwo");
			attr->AddString("attr:name","TAKENOTES:tagthree");
			attr->AddString("attr:public_name","refapp");
			attr->AddString("attr:public_name","type");
			attr->AddString("attr:public_name","tagone");
			attr->AddString("attr:public_name","tagtwo");
			attr->AddString("attr:public_name","tagthree");
			attr->AddInt32("attr:type",B_STRING_TYPE);
			attr->AddInt32("attr:type",B_STRING_TYPE);
 			attr->AddInt32("attr:type",B_STRING_TYPE);
 			attr->AddInt32("attr:type",B_STRING_TYPE);
			attr->AddInt32("attr:type",B_STRING_TYPE);
			attr->AddBool("attr:editable",true);
			attr->AddBool("attr:editable",true);
			attr->AddBool("attr:editable",true);
			attr->AddBool("attr:editable",true);
			attr->AddBool("attr:editable",true);


		
			//attr->PrintToStream();
		
			if (takenotes.SetAttrInfo(attr) != B_OK){
					printf("errore nel settare i metadata\n");
			}
				
			//Add a short description for the MIME type
			if (takenotes.SetShortDescription("TakeNotes") != B_OK)
					printf("errore nel settare la short description\n");
				
			//Finally we install the TakeNotes MIME type
			if (takenotes.Install() != B_OK)
					printf("non installato, errore\n");
		
			delete msg;
			delete attr;
		
		} else {
		
				printf("errore di init del mimetype");
		}


}

void NoteApplication :: OpenNote(){

		
		new NoteWindow(fWindowCountUntitled++);
		fWindowCount++;

}

void NoteApplication :: OpenNote(entry_ref *ref){


		new NoteWindow(ref);
		fWindowCount++;

}

void NoteApplication :: CloseNote(){

		fWindowCount--;
		if (fWindowCount == 0){
		
			BAutolock lock(this);
			Quit();		
		}

} 


void NoteApplication :: ArgvReceived(int32 argc, char** argv){

		//Variables
		const 	char* 		cwd = "";
				BMessage 	*message = CurrentMessage();
		
		
		//Extract the cwd (current working directory)
		if (message != NULL){
		
			if (message->FindString("cwd",&cwd) != B_OK)
					cwd = "";
		}
		
		//Check if it is an absolute or relative path
		//If the path is a relative one we make it absolute
		//Eventually we open the note(s)
		for (int i=1; i < argc; i++){
		
			//Variables
			BPath path;
			entry_ref ref;
			
			if (argv[i][0] == '/'){
				
				path.SetTo(argv[i]);
			
			} else {
			
				path.SetTo(cwd, argv[i]);
			}
			
			get_ref_for_path(path.Path(), &ref);
			OpenNote(&ref);
			
		}

}

void NoteApplication :: RefsReceived(BMessage *message){

		//Variables
		int32 index = 0;
		entry_ref ref;
		
		while(message->FindRef("refs", index++, &ref) == B_OK){
		
			OpenNote(&ref);
		
		}
}


void NoteApplication :: MessageReceived(BMessage *message){


		switch(message->what){
		
		
			case B_SILENT_RELAUNCH:
				OpenNote();
				break;
			
			default:
				BApplication::MessageReceived(message);
				break;
		
		}

}

// Main
int main(){

	NoteApplication myApp;
	
	myApp.Run();
	return(0);
}		
	//
//	// Variables
//	BRect	aRect;
//	
//	// Creation of the rectangle that gives dimensions to the window
//	// x, y, w, h
//	aRect.Set(30, 30, 300, 300);
//	fNoteWindow = new NoteWindow(aRect);	
//	
//	// Show the window
//	fNoteWindow->Show();	


