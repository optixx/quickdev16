#import "AppController.h"

@implementation AppController
- (id) init {
	[super init];
	NSLog(@"init");
	speechSynth = [[NSSpeechSynthesizer alloc] initWithVoice:nil];
	[speechSynth setDelegate:self]; 
	return self;
}

- (void)awakeFromNib{
	NSColor *initialColor = [ textField textColor];
	NSLog(@"setting init color %@",initialColor);
	[colorWell setColor:initialColor];
	NSString *defaultVoice = [NSSpeechSynthesizer defaultVoice];
	NSArray *voices = [NSSpeechSynthesizer availableVoices];
	int defaultRow = [voices indexOfObject:defaultVoice];
	[tableView selectRow:defaultRow byExtendingSelection:NO];
	[tableView scrollRowToVisible:defaultRow];
	 
}
- (IBAction)sayIt:(id)sender {
	NSString *string = [textField stringValue];
	if ( [string length] == 0) {
		NSLog(@"No message");
		return;
	}
	[speechSynth startSpeakingString:string];
	NSLog(@"Have started speaking: %@", string);
    [stopButton setEnabled:YES];
}

- (IBAction)stopIt:(id)sender {
	NSLog(@"stopping");
	[speechSynth stopSpeaking];
    
}

- (void)speechSynthesizer:(NSSpeechSynthesizer *)send
		didFinishSpeaking:(BOOL)finishedSpeaking
{
	NSLog(@"didFinished=%d",finishedSpeaking);
    [stopButton setEnabled:NO];

}


- (IBAction)changeTextColor:(id)sender{
	NSColor *newColor = [sender color];
	NSLog(@"Change Color %@",newColor);
	[textField setTextColor:newColor];
	
}

- (int)numberOfRowsInTableView:(NSTableView *) aTableView{
	NSLog(@"numberOfRowsInTableView %d", [[NSSpeechSynthesizer availableVoices] count]);

	return [[NSSpeechSynthesizer availableVoices] count];
}
-(id)tableView:(NSTableView*) aTableView
			objectValueForTableColumn:(NSTableColumn *) aTableColumn row:(int)row{
	NSString *voice = [[NSSpeechSynthesizer availableVoices] objectAtIndex:row ];
	return [[NSSpeechSynthesizer attributesForVoice:voice] valueForKey:NSVoiceName];
}

- (void)tableViewSelectionDidChange:(NSNotification *)nofication{
	NSArray *availableVoices = [NSSpeechSynthesizer availableVoices];
	int row = [tableView selectedRow];
	if ( row == -1) {
		return;
	}
	
	NSString *selectedVoice = [availableVoices objectAtIndex:row];
	[speechSynth setVoice:selectedVoice];
	NSLog(@"new voice=%@",selectedVoice);
}

-(BOOL)selectionShouldChangeInTableView:(NSTableView *) aTableView
{
	if ([speechSynth isSpeaking]){
		NSBeep();
		return NO;
	} else {
		return YES;
	}
}

-(NSSize)windowWillResize:(NSWindow *)send toSize:(NSSize) framesize;
{
	float w = framesize.width;
	float h = framesize.height;
	NSLog(@"called willResize %f x %f ",w,h);
	
	w = w*2;
	framesize.width = w;
	return framesize;
}

- (void) dealloc {
	NSLog(@"dealloc");
	[speechSynth release];
	[super dealloc];
	
}
@end
