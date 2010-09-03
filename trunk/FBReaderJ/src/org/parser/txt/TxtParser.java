package org.parser.txt;

import java.io.InputStream;

import org.geometerplus.zlibrary.core.txt.ZLTxtReader;

import android.util.Log;

public final class TxtParser {
	static {
		System.loadLibrary("TxtParser");		
	}
	protected String strFilePath;
	protected ZLTxtReader txtReader;
	///////////////////////////////////////////////////////////////////
	private static native void Parser(TxtParser parser, String strPath);
	/////////////////////////////////////////////////////////////////////
	public TxtParser(ZLTxtReader txtReader, String strPath) {
		strFilePath = new String(strPath);
		this.txtReader = txtReader;
	}		
	public void Check(int nLen) {
		Parser(this, new String("123"));
	}	
	public void doIt() {
		if (strFilePath != null) {
			Parser(this, strFilePath);
		}		
	}
	public void characterDataHandler(String strContent) {
		Log.d("TEXT Reader:", strContent);
		//byte[] bContent = tag.getBytes();
		// addByteData(bContent, 0, bContent.length);
		txtReader.characterDataHandler(new String(strContent));
	}

	public void startElementHandler() {
		// startNewParagraph();
		Log.d("TEXT Reader:", "startElementHandler");
		txtReader.startElementHandler();
	}

	public void endElementHandler() {
		// endParagraph();
		Log.d("TEXT Reader:", "endElementHandler");
		txtReader.endElementHandler();

	}
}
