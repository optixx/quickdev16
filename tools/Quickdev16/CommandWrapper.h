//
//  CommandWrapper.h
//  Quickdev16
//
//  Created by David Voswinkel on 09-09-01.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface CommandWrapper : NSObject {

    
    IBOutlet NSTextField *textField;
}
- (void)doCommand;
- (void)doPipedCommand;

@end
