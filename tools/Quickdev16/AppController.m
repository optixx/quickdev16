#import "AppController.h"
#import "CommandWrapper.h"

#define UCON64 "/usr/local/bin/ucon64"
#define ROMFILE "/Users/david/Devel/arch/avr/code/quickdev16/roms/mrdo.smc"


@implementation AppController
- (id) init {
	[super init];
	NSLog(@"init");
	return self;
}

- (void)awakeFromNib {
    [textFieldLog setStringValue:@"Log field"];
	[textFieldInfo setStringValue:@"Info field"];

	NSLog(@"awakeFromNib");
}

- (IBAction)romUpload:(id)sender {
	NSLog(@"romUpload:");
    NSTask *ls=[[NSTask alloc] init];
    NSPipe *pipe=[[NSPipe alloc] init];
    NSFileHandle *handle;
    
    [ls setLaunchPath:@UCON64];
    [ls setArguments:[NSArray arrayWithObjects:@"-smc",@"--port=usb",@"--xquickdev16",@ROMFILE,nil]];
    [ls setStandardOutput:pipe];
    handle=[pipe fileHandleForReading];
    
    [ls launch];
    
    [NSThread detachNewThreadSelector:@selector(copyData:)
							 toTarget:self withObject:handle];
    
    [pipe release];
    [ls release];
}


- (void)copyData:(NSFileHandle*)handle {
    NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
    NSData *data;
	
    while([data=[handle availableData] length]) { // until EOF (check reference)
        NSString *string=[[NSString alloc] initWithData:data
											   encoding:NSASCIIStringEncoding];
        
		[textFieldLog setStringValue:string];
		[string release];
    }
	
    [pool release];
}



- (IBAction)romInfo:(id)sender {
    NSTask *ls=[[NSTask alloc] init];
    NSPipe *pipe=[[NSPipe alloc] init];
    NSFileHandle *handle;
    NSString *string;
	NSLog(@"romInfo");
    
    [ls setLaunchPath:@"/usr/local/bin/ucon64"];
    [ls setArguments:[NSArray arrayWithObjects:@ROMFILE,nil]];
    [ls setStandardOutput:pipe];
    handle=[pipe fileHandleForReading];
    
    [ls launch];
    
    string=[[NSString alloc] initWithData:[handle readDataToEndOfFile]
								 encoding:NSASCIIStringEncoding]; // convert NSData -> NSString
	
 	NSLog(@"romInfo: %@", string);
    [textFieldInfo setStringValue:string];
    [string retain];
    [pipe release];
    [ls release];
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
