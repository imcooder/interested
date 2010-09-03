
package com.hanvon;

import java.util.*;
import java.lang.System;
import java.lang.String;


public final class Trace{
	Trace(){
		
	}	
	final static long nLevel;
	static {
		nLevel = (long)(-1);
	}
	public static void DBGMSG(long level, String strFormat, Object...args) {
		if(0 == (Trace.nLevel & level)) {
			return;
		}
		String strTrace = new String();
		strTrace = String.format(strFormat, args);		
		android.util.Log.d("TRACE", strTrace);
		return;
	}	
	public static void RETAILMSG(long level, String strFormat, Object...args) {
		if(0 == (level & Trace.nLevel)) {
			return;
		}
		String strTrace = new String();
		strTrace = String.format(strFormat, args);		
		android.util.Log.d("TRACE", strTrace);
		return;
	}	
}
