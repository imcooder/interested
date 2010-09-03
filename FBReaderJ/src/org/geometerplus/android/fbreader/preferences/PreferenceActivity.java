/*
 * Copyright (C) 2009-2010 Geometer Plus <contact@geometerplus.com>
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

package org.geometerplus.android.fbreader.preferences;

import java.util.List;

import android.content.Context;

import org.geometerplus.zlibrary.core.resources.ZLResource;

import org.geometerplus.zlibrary.ui.android.library.ZLAndroidApplication;

import org.geometerplus.fbreader.fbreader.*;
import org.geometerplus.fbreader.Paths;

public class PreferenceActivity extends ZLPreferenceActivity {
	public PreferenceActivity() {
		super("Preferences");
	}

	private static final class ColorProfilePreference extends ZLSimplePreference {
		private final FBReader myFBReader;
		private final Screen myScreen;
		private final String myKey;

		static final String createTitle(ZLResource resource, String resourceKey) {
			final ZLResource r = resource.getResource(resourceKey);
			return r.hasValue() ? r.getValue() : resourceKey;
		}

		ColorProfilePreference(Context context, FBReader fbreader, Screen screen, String key, String title) {
			super(context);
			myFBReader = fbreader;
			myScreen = screen;
			myKey = key;
			setTitle(title);
		}

		@Override
		public void onAccept() {
		}

		@Override
		public void onClick() {
			myScreen.setSummary(getTitle());
			myFBReader.setColorProfileName(myKey);
			myScreen.close();
		}
	}

	@Override
	protected void init() {
		final Category libraryCategory = createCategory("Library");
		/*
		libraryCategory.addPreference(new InfoPreference(
			this,
			libraryCategory.Resource.getResource("path").getValue(),
			Constants.BOOKS_DIRECTORY)
		);
		*/
		libraryCategory.addPreference(new ZLStringOptionPreference(
			this,
			Paths.BooksDirectoryOption,
			libraryCategory.Resource,
			"path")
		);
		final Category lookNFeelCategory = createCategory("LookNFeel");
		lookNFeelCategory.addOption(ZLAndroidApplication.Instance().AutoOrientationOption, "autoOrientation");
		lookNFeelCategory.addOption(ZLAndroidApplication.Instance().ShowStatusBarOption, "showStatusBar");
		lookNFeelCategory.addOption(ZLAndroidApplication.Instance().DontTurnScreenOffOption, "dontTurnScreenOff");
		lookNFeelCategory.addPreference(new ScrollbarTypePreference(this, lookNFeelCategory.Resource, "scrollbarType"));

		/*
		final FBReader fbreader = (FBReader)FBReader.Instance();
		final Screen colorProfileScreen = lookNFeelCategory.createPreferenceScreen("colorProfile");
		final Category colorProfileCategory = colorProfileScreen.createCategory(null);
		final ZLResource resource = colorProfileCategory.Resource;
		colorProfileScreen.setSummary(ColorProfilePreference.createTitle(resource, fbreader.getColorProfileName()));
		for (String key : ColorProfile.names()) {
			colorProfileCategory.addPreference(new ColorProfilePreference(
				this, fbreader, colorProfileScreen, key, ColorProfilePreference.createTitle(resource, key)
			));
		}
		*/

		final Category scrollingCategory = createCategory("Scrolling");
		final ScrollingPreferences scrollingPreferences = ScrollingPreferences.Instance();
		scrollingCategory.addOption(scrollingPreferences.FlickOption, "flick");
		scrollingCategory.addOption(scrollingPreferences.VolumeKeysOption, "volumeKeys");
		scrollingCategory.addOption(scrollingPreferences.AnimateOption, "animated");
		scrollingCategory.addOption(scrollingPreferences.HorizontalOption, "horizontal");
		
		final Category autoPage = createCategory("AutoPage");		
		autoPage.addPreference(new AutoPagePreference(this, autoPage.Resource, "AutoPage"));
		
	}
}

class ScrollbarTypePreference extends ZLStringListPreference {
	private static final String[] ourCodes = { "hide", "show", "showAsProgress" };

	private FBReader myReader;

	ScrollbarTypePreference(Context context, ZLResource rootResource, String resourceKey) {
		super(context, rootResource, resourceKey);
		myReader = (FBReader)FBReader.Instance();
		final String[] names = new String[ourCodes.length];
		final ZLResource r = rootResource.getResource(resourceKey);
		for (int i = 0; i < ourCodes.length; ++i) {
			names[i] = r.getResource(ourCodes[i]).getValue();
		}
		setLists(ourCodes, names);
		setInitialValue(ourCodes[
			Math.max(0, Math.min(ourCodes.length - 1, myReader.ScrollbarTypeOption.getValue()))
		]);
	}

	public void onAccept() {
		final String value = getValue();
		int intValue = 0;
		for (int i = 0; i < ourCodes.length; ++i) {
			if (value == ourCodes[i]) {
				intValue = i;
				break;
			}
		}
		myReader.ScrollbarTypeOption.setValue(intValue);
	}
}


class AutoPagePreference extends ZLStringListPreference {
	
	private FBReader myReader;
	protected String strDisable;
	AutoPagePreference(Context context, ZLResource rootResource, String resourceKey) {
		super(context, rootResource, resourceKey);
		myReader = (FBReader)FBReader.Instance();		
		final String[] names = new String[myReader.autoPageElapse.length + 1];		
		final ZLResource r = rootResource.getResource(resourceKey);
		String unit = r.getResource("unit").getValue();
		strDisable = r.getResource("disable").getValue();
		names[0] = strDisable;
		for (int i = 0; i < myReader.autoPageElapse.length; ++i) {
			names[i + 1] = String.valueOf(myReader.autoPageElapse[i]) + unit;			
		}
		setLists(names, names);
		String initValue =String.valueOf(myReader.AutoPageOption.getValue());
		if(initValue.equals("0")) {
			initValue = strDisable;
		}
		if(true != setInitialValue(initValue)) {
			initValue =  "0" + unit; 
		}
		setInitialValue(initValue);
	}

	public void onAccept() {
		final String value = getValue();
		int intValue = 0;
		if(!value.equals(strDisable)) {
			try{
				intValue =  Integer.valueOf(value).intValue(); 
			} catch(NumberFormatException e){
				
			}	
		}			
		myReader.AutoPageOption.setValue(intValue);
	}
}
