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

package org.geometerplus.fbreader.formats.txt;

import java.util.HashMap;
import java.io.*;
import java.nio.charset.*;

import org.geometerplus.fbreader.bookmodel.BookModel;
import org.geometerplus.fbreader.bookmodel.BookReader;
import org.geometerplus.zlibrary.core.filesystem.ZLFile;
import org.geometerplus.zlibrary.core.txt.*;
import org.geometerplus.zlibrary.text.model.ZLTextParagraph;

import android.util.Log;

public class TxtReader extends BookReader implements ZLTxtReader {
		
	private final char[] SPACE = { ' ' };		

	public TxtReader(BookModel model) throws UnsupportedEncodingException {
		super(model);
		setByteDecoder(model.Book.getEncoding());		
	}


	public boolean readBook() throws IOException {
		return ZLTxtProcessor.read(this, Model.Book.File.getPath());
	}

	public InputStream getInputStream() throws IOException {
		return Model.Book.File.getInputStream();
	}

	public void startDocumentHandler() {
		setMainTextModel();
	}

	public void endDocumentHandler() {
		unsetCurrentTextModel();
	}

	public void byteDataHandler(byte[] data, int start, int length) {
		addByteData(data, start, length);
	}


	private void startNewParagraph() {
		endParagraph();
		beginParagraph(ZLTextParagraph.Kind.TEXT_PARAGRAPH);
	}	
	public void characterDataHandler(String tag) {
		Log.d("TEXT Reader:", tag);
		addData(tag.toCharArray());
		//byte[] bContent = tag.getBytes();
		//addByteData(bContent, 0, bContent.length);		
	}	
	public void startElementHandler() {
		startNewParagraph();
	}

	public void endElementHandler() {
		endParagraph();
	}

	public void setByteDecoder(String strCharset) {
		if (strCharset != null && strCharset.length() > 0) {
			Charset charset = null;
			CharsetDecoder decoder = null;
			try {
				charset = Charset.forName(strCharset);
			} catch (IllegalCharsetNameException e) {

			} catch (UnsupportedCharsetException e) {

			}
			if (charset != null) {
				decoder = charset.newDecoder();
			}
			if (decoder != null) {
				Model.Book.setEncoding(strCharset);
				decoder.onMalformedInput(CodingErrorAction.REPLACE);
				decoder.onUnmappableCharacter(CodingErrorAction.REPLACE);
				setByteDecoder(decoder);
			}
		}

	}
}
