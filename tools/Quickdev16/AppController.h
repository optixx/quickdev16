#import <Cocoa/Cocoa.h>

@interface AppController : NSObject  {
    IBOutlet NSTextField *textFieldLog;
	IBOutlet NSTextField *textFieldInfo;

}
- (IBAction)romInfo:(id)sender;
- (IBAction)romUpload:(id)sender;


@end
