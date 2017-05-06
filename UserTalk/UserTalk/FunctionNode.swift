//
//  FunctionNode.swift
//  UserTalk
//
//  Created by Brent Simmons on 5/4/17.
//  Copyright © 2017 Ranchero Software. All rights reserved.
//

import Foundation
import FrontierData

final class FunctionNode: CodeTreeNode {

	let operation: CodeTreeOperation = .functionOp
	let textPosition: TextPosition
	let name: String
	let params: [FunctionHeaderParamNode]
	let blockNode: BlockNode
	
	init(_ textPosition: TextPosition, name: String, params: [Params], blockNode: BlockNode) {
		
	}
}
