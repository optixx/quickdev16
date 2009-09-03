//
//  CommandWrapper.m
//  Quickdev16
//
//  Created by David Voswinkel on 09-09-01.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "CommandWrapper.h"


@implementation CommandWrapper

- (void)doCommand {
    
	NSTask *command=[[NSTask alloc] init];
    
    [command setLaunchPath:@"/bin/ls"];
    [command setArguments:[NSArray arrayWithObjects:@"-l",@"/System",nil]];
    [command launch];
    
    [command release];
}



- (void)doPipedCommand {
    NSTask *ls=[[NSTask alloc] init];
    NSPipe *pipe=[[NSPipe alloc] init];
    NSFileHandle *handle;
    NSString *string;
    
    [ls setLaunchPath:@"/bin/ls"];
    [ls setArguments:[NSArray arrayWithObjects:@"-l",@"/System",nil]];
    [ls setStandardOutput:pipe];
    handle=[pipe fileHandleForReading];
    
    [ls launch];
    
    string=[[NSString alloc] initWithData:[handle readDataToEndOfFile]
								 encoding:NSASCIIStringEncoding]; // convert NSData -> NSString
   
	
 	NSLog(@"doPipedCommand: %@", string);
    [textField setStringValue:string];
    
    [string release];
    [pipe release];
    [ls release];
}

@end





