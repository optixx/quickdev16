#import "AppController.h"
#import "CommandWrapper.h"

@implementation AppController
- (id) init {
	[super init];
	NSLog(@"init");
	return self;
}

- (void)awakeFromNib{
	NSLog(@"awakeFromNib");
}

- (IBAction)romInfo:(id)sender {
	NSString *string = [textField stringValue];
	if ( [string length] == 0) {
		NSLog(@"No message");
		return;
	}
	NSLog(@"Have started speaking: %@", string);
    //[stopButton setEnabled:YES];
}

- (IBAction)romUpload:(id)sender {
	NSLog(@"romUpload");
    
	CommandWrapper *cw=[[CommandWrapper alloc] init];
    [cw doPipedCommand];
	
	
	NSString* myString = [[NSString alloc] init];
	myString = @"test"; 
    [textField setStringValue:myString];
	if ( [myString length] != 0) {
		NSLog(@"message: %@", myString);
		return;
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
	[super dealloc];
	
}
@end
