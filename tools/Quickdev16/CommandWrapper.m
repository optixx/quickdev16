//
//  CommandWrapper.m
//  Quickdev16
//
//  Created by David Voswinkel on 09-09-01.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "CommandWrapper.h"


@implementation CommandWrapper


- (void)awakeFromNib {
	NSLog(@"awakeFromNib");
}

- (void)doCommand {
	NSTask *command=[[NSTask alloc] init];
    [command setLaunchPath:@"/bin/ls"];
    [command setArguments:[NSArray arrayWithObjects:@"-l",@"/System",nil]];
    [command launch];
    [command release];
}

- (NSString *)doPipedCommand {
    NSTask *ls=[[NSTask alloc] init];
    NSPipe *pipe=[[NSPipe alloc] init];
    NSFileHandle *handle;
    NSString *string;
    
    [ls setLaunchPath:@"/usr/local/bin/ucon64"];
    [ls setArguments:[NSArray arrayWithObjects:@"/Users/david/Devel/arch/avr/code/quickdev16/roms/super01.smc",nil]];
    [ls setStandardOutput:pipe];
    handle=[pipe fileHandleForReading];
    
    [ls launch];
    
    string=[[NSString alloc] initWithData:[handle readDataToEndOfFile]
								 encoding:NSASCIIStringEncoding]; // convert NSData -> NSString
   
 	NSLog(@"doPipedCommand: %@", string);
    //[string retain];
    [pipe release];
    [ls release];
	return string;
}


@end





