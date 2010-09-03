/*
 * Copyright (C) 2007-2010 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

package org.geometerplus.zlibrary.text.view;

import org.geometerplus.zlibrary.core.view.ZLPaintContext;
import com.hanvon.*;
public final class ZLTextWord extends ZLTextElement { 
	public final char[] Data;
	public final int Offset;
	public final int Length;
	private int myWidth = -1;
	private Mark myMark;
	private int myParagraphOffset;
	public byte[] marktype = null;
	
	interface MarkType{
		int TYPE_UNDERLINE = 0x1;
		int TYPE_HIGHLINGHT = 0x2;
		int TYPE_HIGHLINGHT_SEARCH = 0x4;		 
	}
	class Mark {
		
		public final int Start;
		public final int Length;
		public int Type;
		private Mark myNext;

		private Mark(int start, int length, int type) {
			Start = start;
			Length = length;
			Type = type;
			myNext = null;
		}

		public Mark getNext() {
			return myNext;
		}

		private void setNext(Mark mark) {
			myNext = mark;
		}
	}
	
	public ZLTextWord(char[] data, int offset, int length, int paragraphOffset) {
		Data = data;
		Offset = offset;
		Length = length;
		if(Length > 0) {
			marktype = new byte[Length];
			if(marktype != null) {
				for(int nIdx = 0; nIdx < marktype.length; nIdx ++) {
					marktype[nIdx] = 0;
				}
			}
		}
		myParagraphOffset = paragraphOffset;
	}

	public Mark getMark() {
		return myMark;
	}

	public int getParagraphOffset() {
		return myParagraphOffset;
	}
	
	public void addMark(int start, int length, int type) {
		Mark existingMark = myMark;
		Mark mark = new Mark(start, length, type);
		if ((existingMark == null) || (existingMark.Start > start)) {
			mark.setNext(existingMark);
			myMark = mark;
		} else {
			while ((existingMark.getNext() != null) && (existingMark.getNext().Start < start)) {
				existingMark = existingMark.getNext();
			}
			mark.setNext(existingMark.getNext());
			existingMark.setNext(mark);
		}
		
		///
		Trace.DBGMSG(1, "addMark %d %d %d\n", start, start + length, marktype.length);
		for(int nIdx = start; nIdx < Math.min(start + length, marktype.length); nIdx ++) {
			marktype[nIdx] |= type;
		}
		
	}
	public int getWidth(ZLPaintContext context) {
		int width = myWidth;
		if (width == -1) {
			width = context.getStringWidth(Data, Offset, Length);	
			myWidth = width;
		}
		return width;
	}

	public String toString() {
		StringBuffer sb = new StringBuffer();
		for (int i = Offset; i < Offset + Length; i++) {
			sb.append(Data[i]);
		}	
		return sb.toString();
	}
}
