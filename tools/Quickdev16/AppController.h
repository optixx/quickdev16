#import <Cocoa/Cocoa.h>

@interface AppController : NSObject  {
    IBOutlet NSTextField *textField;
	IBOutlet NSColorWell *colorWell;
	IBOutlet NSButton *stopButton;
	IBOutlet NSTableView *tableView;
	NSSpeechSynthesizer *speechSynth;
}
- (IBAction)sayIt:(id)sender;
- (IBAction)stopIt:(id)sender;
- (IBAction)changeTextColor:(id)sender;
- (void)speechSynthesizer:(NSSpeechSynthesizer *)send
		didFinishSpeaking:(BOOL)finishedSpeaking;
- (int)numberOfRowsInTableView:(NSTableView *) aTableView;
-(id)tableView:(NSTableView*) aTableView
objectValueForTableColumn:(NSTableColumn *) aTableColumn row:(int)row;

@end
