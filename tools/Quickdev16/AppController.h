#import <Cocoa/Cocoa.h>

@interface AppController : NSObject  {
    IBOutlet NSTextField *textField;
}
- (IBAction)romInfo:(id)sender;
- (IBAction)romUpload:(id)sender;


@end
