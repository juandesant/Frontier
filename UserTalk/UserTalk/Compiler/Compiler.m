//
//  Compiler.m
//  UserTalk
//
//  Created by Brent Simmons on 4/30/17.
//  Copyright © 2017 Ranchero Software. All rights reserved.
//

#import "Compiler.h"
#import <UserTalk/UserTalk-Swift.h>

@class CodeTreeNode;

@interface Compiler : NSObject

@property (nonatomic) NSString *text;
@property (nonatomic) BOOL lineBased;
@property (nonatomic) Tokenizer *tokenizer;
@property (nonatomic) CodeTreeNode *compiledCode;

@end


static NSLock *lock = nil;

@implementation Compiler

- (instancetype)initWithText:(NSString *)text lineBased:(BOOL)lineBased {

	self = [super init];
	if (!self) {
		return nil;
	}

	_text = text;
	_lineBased = lineBased;
	return self;
}


- (void)compile {

	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		lock = [[NSLock alloc] init];
	});

	// Communicating with the Bison-generated parser uses some globals, so we have to lock while compiling.

	[lock lock];

	self.tokenizer = [[Tokenizer alloc] initWithText:self.text lineBasedScan:self.lineBased];

//	NSInteger parseResult = yyparse();
	yyparse();
	self.compiledCode = yyval;

	[lock unlock];
}

@end

CodeTreeNode *compile(NSString *text, BOOL lineBased) {

	Compiler *compiler = [[Compiler alloc] initWithText:text lineBased:lineBased];
	[compiler compile];
	return compiler.compiledCode;
}


// PBS 30 April 2017: The Bison-generated parser calls these as it parses.
// These C functions bridge the parser, which is C code, and CodeTreeNode, which is Swift.

CodeTreeNode *pushquadruplet (NSInteger nodeType, CodeTreeNode *p1, CodeTreeNode *p2, CodeTreeNode *p3, CodeTreeNode *p4) {

	return [[CodeTreeNode alloc] initWithNodeType:nodeType param1:p1 param2:p2 param3:p3 param4:p4];
}

CodeTreeNode *pushoperation(NSInteger type) {
	
	return pushquadruplet(type, nil, nil, nil, nil);
}

CodeTreeNode *pushunaryoperation(NSInteger type, CodeTreeNode *param1) {

	return pushquadruplet(type, param1, nil, nil, nil);
}

CodeTreeNode *pushbinaryoperation(NSInteger type, CodeTreeNode *param1, CodeTreeNode *param2) {

	return pushquadruplet(type, param1, param2, nil, nil);
}

void pushlastlink(CodeTreeNode *newLast, CodeTreeNode *list) {

	/*
	 newlast is a newly parsed object, for example a statement in a statement
	 list or a parameter expression in a parameter list.

	 tack it on at the end of the list headed up by hlist. we travel from
	 list until we hit nil, then backup and insert newlast as the last node
	 in the list.

	 the link field of the CodeTreeNode is used to connect the list.

	 12/9/91 dmb: don't treat nil newlast as an error
	 */

	if (!newLast || !list) {
		return;
	}

	CodeTreeNode *nomad = list;

	while(true) {

		if (!nomad.link) {
			nomad.link = newLast;
			return;
		}
		nomad = nomad.link;
	}
}

void pushtripletstatementlists(CodeTreeNode *p2, CodeTreeNode *p3, CodeTreeNode *list) {

	/*
	 add the statement lists to the previously-created triplet.

	 the triplet was created earlier so that the line/character position
	 information could be meaningful.

	 since the triplet was created with unassigned parameters set to
	 nil, we only set non-nil parameters.  this is more than an optimization;
	 it adds flexibility so that for some callers (like fileloop), one
	 of these parameters can already be set
	 */

	if (p2) {
		list.param2 = p2;
	}
	if (p3) {
		list.param3 = p3;
	}
}

void pushunarystatementlist(CodeTreeNode *p1, CodeTreeNode *list) {

	list.param1 = p1;
}

void pushloopbody(CodeTreeNode *p4, CodeTreeNode *list) {

	list.param4 = p4;
}

CodeTreeNode *pushkernelcall(CodeTreeNode *node) {

	return [[CodeTreeNode alloc] initWithNodeType:kernelOptionsErr takingValueFromNode:node];
}

CodeTreeNode *pushtriplet(NSInteger nodeType, CodeTreeNode *p1, CodeTreeNode *p2, CodeTreeNode *p3) {
	
	return pushquadruplet(nodeType, p1, p2, p3, nil);
}

CodeTreeNode *pushloop(CodeTreeNode *p1, CodeTreeNode *p2, CodeTreeNode *p3) {

	// Objective-C can’t see Ints from CodeTreeNodeType.
	return pushquadruplet(24 /*loopOp*/, p1, p2, p3, nil);
}

