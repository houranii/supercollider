
// These Streams are instantiated by math operations on other Streams

UnaryOpStream : Stream {	
	var >operator, >a;
	
	*new { arg operator, a;
		^super.newCopyArgs(operator, a)
	}
	next {
		var vala;
		vala = a.next;
		if (vala.isNil, { ^nil },{ ^vala.perform(operator); });
	}
	reset { a.reset }
}

BinaryOpStream : Stream {	
	var >operator, >a, >b;
	
	*new { arg operator, a, b;	
		^super.newCopyArgs(operator, a, b)
	}
	next {  
		var vala, valb;
		vala = a.next;
		if (vala.isNil, { ^nil });
		valb = b.next;
		if (valb.isNil, { ^nil });
		
		^vala.perform(operator, valb);
	}
	reset { a.reset; b.reset }
}

