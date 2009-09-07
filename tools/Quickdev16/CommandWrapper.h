//
//  CommandWrapper.h
//  Quickdev16
//
//  Created by David Voswinkel on 09-09-01.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface CommandWrapper : NSObject {
}
- (void)doCommand;
- (NSString *)doPipedCommand;
- (void)doThreadedCommand;

@end
