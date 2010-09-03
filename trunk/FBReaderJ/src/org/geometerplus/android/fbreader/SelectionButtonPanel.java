

package org.geometerplus.android.fbreader;

import java.util.ArrayList;

import android.os.Handler;
import android.os.Message;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.animation.AlphaAnimation;
import android.widget.*;
import android.widget.LinearLayout.*;
import android.view.*;

import org.geometerplus.fbreader.fbreader.ActionCode;
import org.geometerplus.zlibrary.ui.android.R;

import org.geometerplus.zlibrary.core.application.ZLApplication;
import org.geometerplus.zlibrary.core.resources.ZLResource;

public class SelectionButtonPanel implements ZLApplication.ButtonPanel, View.OnClickListener {
	boolean Visible = false;
	private View myParentView = null;
	private ArrayList<View> myButtons = new ArrayList<View>();
	private LinearLayout myPlateLayout = null;	
	protected PopupWindow myPopupWindow = null;
	protected View myView = null;
	
	
	class TextActionButton extends Button {
		final String ActionId;
		final boolean IsCloseButton;

		TextActionButton(Context context, String actionId, boolean isCloseButton) {
			super(context);
			ActionId = actionId;
			IsCloseButton = isCloseButton;
		}
	}
	
	class ImageActionButton extends ZoomButton {
		final String ActionId;
		final boolean IsCloseButton;

		ImageActionButton(Context context, String actionId, boolean isCloseButton) {
			super(context);
			ActionId = actionId;
			IsCloseButton = isCloseButton;
		}
	}
	public SelectionButtonPanel(View parent) {		
		myParentView = parent;		
		final LayoutInflater inflater =
			(LayoutInflater)myParentView.getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		myView = inflater.inflate(R.layout.selection_panel, null);
		myPlateLayout = (LinearLayout)myView.findViewById(R.id.selection_controlpanel);		
	}
	public void onClick(View view) {	
		String strActionID = null;
		boolean blIsClose = false;
		if (view instanceof TextActionButton) {
			TextActionButton txtButton = (TextActionButton)view;
			blIsClose = txtButton.IsCloseButton;
			strActionID = txtButton.ActionId;			
		} else if(view instanceof ImageActionButton) {			
			ImageActionButton imgButton = (ImageActionButton)view;
			blIsClose = imgButton.IsCloseButton;
			strActionID = imgButton.ActionId;
		}	
		if(strActionID != null) {
			ZLApplication.Instance().doAction(strActionID);
			if (blIsClose) {
				hide(true);
			}	
		}		
	}
	public void addButton(String actionId, boolean isCloseButton, int imageId) {
		final ImageActionButton button = new ImageActionButton(myParentView.getContext(), actionId, isCloseButton);		
		button.setOnClickListener(this);
		myPlateLayout.addView(button);
		myButtons.add(button);
	}	
	public void addButton(String actionId, boolean isCloseButton, String title) {
		final TextActionButton button = new TextActionButton(myParentView.getContext(), actionId, isCloseButton);		
		button.setText(title.subSequence(0, title.length()));		
		button.setOnClickListener(this);
		myPlateLayout.addView(button);
		myButtons.add(button);
	}
	public boolean onTouchEvent(MotionEvent event) {
		return true;
	}

	private interface VisibilityAction {
		int SHOW_ANIMATED = 0;
		int SHOW_INSTANTLY = 1;
		int HIDE_ANIMATED = 2;
		int HIDE_INSTANTLY = 3;
	}
	
	private Handler myVisibilityHandler = new Handler() {
		public void handleMessage(Message message) {
			switch (message.what) {
				case VisibilityAction.SHOW_ANIMATED:
					fade(View.VISIBLE, 0.0f, 1.0f);
					break;
				case VisibilityAction.SHOW_INSTANTLY:
					myPlateLayout.setVisibility(View.VISIBLE);
					break;
				case VisibilityAction.HIDE_ANIMATED:
					fade(View.GONE, 1.0f, 0.0f);
					hide();
					break;
				case VisibilityAction.HIDE_INSTANTLY:
					myPlateLayout.setVisibility(View.GONE);
					hide();
					break;
			}
		}
	};

	public void show(boolean animate) {
		if(myPopupWindow == null) {
			myPopupWindow = new PopupWindow(myView,  
	                LayoutParams.FILL_PARENT, LayoutParams.WRAP_CONTENT);  			
		}
		if(myPopupWindow != null) {
			myPopupWindow.showAtLocation(myParentView, Gravity.CENTER, 0, 0);
		}
		myVisibilityHandler.sendEmptyMessage(animate ? VisibilityAction.SHOW_ANIMATED : VisibilityAction.SHOW_INSTANTLY);
	}

	public void hide(boolean animate) {
		myVisibilityHandler.sendEmptyMessage(animate ? VisibilityAction.HIDE_ANIMATED : VisibilityAction.HIDE_INSTANTLY);
	}
	
	private void fade(int visibility, float startAlpha, float endAlpha) {
		final AlphaAnimation animation = new AlphaAnimation(startAlpha, endAlpha);
		animation.setDuration(5000);
		myPlateLayout.startAnimation(animation);
		myPlateLayout.setVisibility(visibility);
	}
	
	public void updateStates() {
		final ZLApplication application = ZLApplication.Instance();
		for (View button : myButtons) {
			if (button instanceof TextActionButton) {
				((TextActionButton)button).setEnabled(application.isActionEnabled(((TextActionButton)button).ActionId));
			} else if(button instanceof ImageActionButton) {
				((ImageActionButton)button).setEnabled(application.isActionEnabled(((ImageActionButton)button).ActionId));
			}			
		}
	}
		
	public boolean hasFocus() {
		for (View button : myButtons) {
			if (button.hasFocus()) {
				return true;
				
			}
		}
		return false;
	}
	public void hide() {
		Visible = false;
		if(myPopupWindow != null) {
			myPopupWindow.dismiss();
			myPopupWindow = null;
		}
	}
	public boolean isShowing() {
		return myPopupWindow != null && myPopupWindow.isShowing();
	}
	public void init() {
		ZLResource myResource = ZLResource.resource("SelectionOperator");			
		addButton(ActionCode.SELECTION_UNDERLINE, true, myResource.getResource("underline").getValue());
		addButton(ActionCode.SELECTION_COPYSTRING, true, myResource.getResource("copystring").getValue());
		addButton(ActionCode.SELECTION_TRANSLATE, true, myResource.getResource("translate").getValue());
		addButton(ActionCode.SELECTION_HIGHLIGHT, true, myResource.getResource("highlight").getValue());
	}
}

