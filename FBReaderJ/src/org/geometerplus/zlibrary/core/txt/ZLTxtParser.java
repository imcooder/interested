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

package org.geometerplus.zlibrary.core.txt;

import java.io.*;
import java.util.*;
import org.geometerplus.zlibrary.core.util.*;

import org.geometerplus.zlibrary.core.util.ZLArrayUtils;
import org.geometerplus.zlibrary.core.txt.ZLTxtReader;
import org.geometerplus.zlibrary.core.html.*;
import org.parser.txt.*;

final class ZLTxtParser {
	private final ZLTxtReader myReader;
	protected String filePath;
	public ZLTxtParser(ZLTxtReader txtReader, String path)
			throws IOException {
		myReader = txtReader;		
		filePath= path;
	}

	public void doIt() throws IOException {
		TxtParser a;		
		a = new TxtParser(myReader, new String("/mnt/sdcard/Books/feedbooks.com/userbook/net_b.txt"));	
		a.doIt();
		/*
		final InputStream stream = myStream;
		final ZLTxtReader txtReader = myReader;
		byte[] buffer = new byte[1024 * 8];
		int bufferOffset = 0;
		int offset = 0;
		int paragraphCount = 0;
		int paragraphLen = 0;
		boolean blBookStart = true;
		while (true) {
			final int count = stream.read(buffer);
			if (count <= 0) {
				break;
			} else {
				if (count < buffer.length) {
					buffer = ZLArrayUtils.createCopy(buffer, count, count);
				}
				int startPosition = 0;
				if (paragraphCount <= 0) {
				String txtCoder = "gbk";
				if(buffer.length >= 2) {
					if(buffer.length >= 2 && (buffer[0] == (byte)0xFF && buffer[1] == (byte)0xFE)) {
						txtCoder = "utf-16";
						startPosition += 2;
					}
					else if(buffer.length >= 2 && (buffer[0] == (byte)0xFE && buffer[1] == (byte)0xFF)) {
						txtCoder = "utf-16";
						startPosition += 2;
					}
					else if(buffer.length >= 3 && (buffer[0] == (byte)0xEF && buffer[1] == (byte)0xBB && buffer[2] == (byte)0xBF)){
						txtCoder = "utf-8";
						startPosition += 3;
					}
					else if(buffer.length >= 4 && (buffer[0] == (byte)0xFF && buffer[1] == (byte)0xFE && buffer[2] == (byte)0x00 && buffer[3] == (byte)0x00)) {
						txtCoder = "utf-32le";
						startPosition += 4;
					} 
					else if(buffer.length >= 4 && (buffer[0] == (byte)0x00 && buffer[1] == (byte)0x00 && buffer[2] == (byte)0xFE && buffer[3] == (byte)0xFF)) {
						txtCoder = "utf-32be";
						startPosition += 4;
					}
					myReader.setByteDecoder(txtCoder);
				}
				}

				if (blBookStart) {
					myReader.startElementHandler();
				}
				try {
					for (int nIdx = startPosition; nIdx < count; nIdx++) {
						if (buffer[nIdx] == '\n' || buffer[nIdx] == '\r') {
							boolean blSkipNewLine = false;
							if (buffer[nIdx] == '\r' && nIdx + 1 != count
									&& buffer[nIdx] == '\n') {
								blSkipNewLine = true;
								buffer[nIdx] = '\n';
							}
							if (startPosition != nIdx) {
								// decode
								myReader.characterDataHandler(new String(
										buffer, startPosition, nIdx
												- startPosition));
							}
							if (blSkipNewLine) {
								nIdx++;
							}
							startPosition = nIdx + 1;
							// myReader.newLineHandler();
							myReader.endElementHandler();
							myReader.startElementHandler();
						} else if (false) {
							if (buffer[nIdx] != '\t') {
								buffer[nIdx] = ' ';
							}
						} else {

						}
					}
					if (startPosition != count) {
						// decode
						myReader.characterDataHandler(new String(buffer,
								startPosition, count - startPosition));
					}

				} catch (ArrayIndexOutOfBoundsException e) {

				}
				bufferOffset += count;
			}

		}
		*/
	}
}
