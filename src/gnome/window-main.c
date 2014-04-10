/********************************************************************\
 * MainWindow.c -- the main window, and associated helper functions * 
 *                 and callback functions for xacc (X-Accountant)   *
 * Copyright (C) 1998,1999 Jeremy Collins	                        *
 * Copyright (C) 1998      Linas Vepstas                            *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, write to the Free Software      *
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.        *
\********************************************************************/

#include <gnome.h>
#include <guile/gh.h>

#include "config.h"

#include "gnucash.h"
#include "top-level.h"
#include "messages.h"
#include "version.h"
#include "util.h"

#include "MainWindow.h"
#include "RegWindow.h"
#include "window-main-menu.h"
#include "window-mainP.h"
#include "window-help.h"
#include "dialog-options.h"
#include "AccWindow.h"

#include "g-wrap.h"

/* This static indicates the debugging module that this .o belongs to.  */
static short module = MOD_GUI;

#include "util.h"

/** STRUCTURES ******************************************************/

/** PROTOTYPES ******************************************************/
static void gnc_ui_options_cb( GtkWidget *, gpointer );
static void gnc_ui_add_account( GtkWidget *, gpointer );
static void gnc_ui_delete_account_cb( GtkWidget *, gpointer );
static void gnc_ui_about_cb( GtkWidget *, gpointer );
static void gnc_ui_help_cb( GtkWidget *, gpointer );
static void gnc_ui_reports_cb( GtkWidget *, gchar * );
static void gnc_ui_mainWindow_toolbar_open( GtkWidget *, gpointer );
static void gnc_ui_mainWindow_toolbar_edit( GtkWidget *, gpointer );
static void gnc_ui_refresh_statusbar();

/** GLOBALS *********************************************************/
char	    *HELP_ROOT = "";

/** Menus ***********************************************************/
static GnomeUIInfo filemenu[] = {
       {GNOME_APP_UI_ITEM, 
       N_("New"), N_("Create New File."),
       NULL, NULL, NULL, 
       GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW,
       0, 0, NULL},
       {GNOME_APP_UI_ITEM,
       N_("Open"), N_("Open File."),
       file_cmd_open, NULL, NULL,
       GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_OPEN,
       0,0, NULL},
       {GNOME_APP_UI_ITEM,
       N_("Save"), N_("Save File."),
       file_cmd_save, NULL, NULL,
       GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SAVE,
       0, 0, NULL},
       {GNOME_APP_UI_ITEM,
       N_("Import"), N_("Import QIF File."),
       file_cmd_import, NULL, NULL,
       GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_CONVERT,
       0, 0, NULL},
       GNOMEUIINFO_SEPARATOR,
       {GNOME_APP_UI_ITEM,
       N_("Exit"), N_("Exit Gnucash."),
       gnc_shutdown, NULL, NULL,
       GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_QUIT,
       0, 0, NULL},       
       GNOMEUIINFO_END             
};

static GnomeUIInfo reportsmenu[] = {
	{GNOME_APP_UI_ITEM,
	 N_("Balance"), N_("BalanceReport"),
	 gnc_ui_reports_cb, "report-baln.phtml", NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PREF,
	 0, 0, NULL},
	{GNOME_APP_UI_ITEM,
	 N_("Profit & Loss"), N_("ProfitLoss"),
	 gnc_ui_reports_cb, "report-pnl.phtml", NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PREF,
	 0, 0, NULL},
	 
	 GNOMEUIINFO_END
};

static GnomeUIInfo optionsmenu[] = {
	{GNOME_APP_UI_ITEM,
	 N_("Preferences"), N_("Preferences"),
	 gnc_ui_options_cb, NULL, NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PREF,
	 0, 0, NULL},
	 GNOMEUIINFO_END
};
  
static GnomeUIInfo accountsmenu[] = {
	{GNOME_APP_UI_ITEM,
	 N_("View"), N_("View Account"),
	 gnc_ui_mainWindow_toolbar_open, NULL, NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_OPEN,
	 0, 0, NULL},
	{GNOME_APP_UI_ITEM,
	 N_("Edit"), N_("Edit Account"),
	 gnc_ui_mainWindow_toolbar_edit, NULL, NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PROP,
	 0, 0, NULL},
	GNOMEUIINFO_SEPARATOR,
	{GNOME_APP_UI_ITEM,
	 N_("Add"), N_("Add Account"),
	 gnc_ui_add_account, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL,
	 0, 0, NULL},
	{GNOME_APP_UI_ITEM,
	 N_("Remove"), N_("Remove Account"),
	 gnc_ui_delete_account_cb, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL,
	 0, 0, NULL},
	 GNOMEUIINFO_END
};  
  
static GnomeUIInfo helpmenu[] = {
    {GNOME_APP_UI_ITEM, 
     N_("About"), N_("About Gnucash."),
     gnc_ui_about_cb, NULL, NULL, 
     GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT, 0, 0, NULL},
     GNOMEUIINFO_SEPARATOR,
    {GNOME_APP_UI_ITEM,
     N_("Help"), N_("Gnucash Help."),
     gnc_ui_help_cb, NULL, NULL,
     GNOME_APP_PIXMAP_NONE, NULL,
     0, 0, NULL},
     GNOMEUIINFO_END
};

static GnomeUIInfo scriptsmenu[] = {
  GNOMEUIINFO_END
};

static GnomeUIInfo mainmenu[] = {
    GNOMEUIINFO_SUBTREE(N_("File"), filemenu),
    GNOMEUIINFO_SUBTREE(N_("Accounts"), accountsmenu),
    GNOMEUIINFO_SUBTREE(N_("Reports"), reportsmenu),
    GNOMEUIINFO_SUBTREE(N_("Options"), optionsmenu),
    GNOMEUIINFO_SUBTREE(N_("Extensions"), scriptsmenu),
    GNOMEUIINFO_SUBTREE(N_("Help"), helpmenu),
    GNOMEUIINFO_END
};

/** TOOLBAR ************************************************************/
GnomeUIInfo toolbar[] = 
{
  { GNOME_APP_UI_ITEM,
    N_("Open"), 
    N_("Open File."),
    file_cmd_open, 
    NULL, 
    NULL,
    GNOME_APP_PIXMAP_STOCK, 
    GNOME_STOCK_PIXMAP_OPEN, 'o', (GDK_CONTROL_MASK), NULL
  },
  { GNOME_APP_UI_ITEM,
    N_("Save"), 
    N_("Save File."),
    file_cmd_save, 
    NULL, 
    NULL,
    GNOME_APP_PIXMAP_STOCK, 
    GNOME_STOCK_PIXMAP_SAVE, 's', (GDK_CONTROL_MASK), NULL
  },
  { GNOME_APP_UI_ITEM,
    N_("Import"), 
    N_("Import QIF File."),
    file_cmd_import, 
    NULL, 
    NULL,
    GNOME_APP_PIXMAP_STOCK, 
    GNOME_STOCK_PIXMAP_CONVERT, 'i', (GDK_CONTROL_MASK), NULL
  },
  GNOMEUIINFO_SEPARATOR,
  { GNOME_APP_UI_ITEM, 
    N_("View"), 
    N_("View selected account."),
    gnc_ui_mainWindow_toolbar_open, 
    NULL,
    NULL,
    GNOME_APP_PIXMAP_STOCK,
    GNOME_STOCK_PIXMAP_OPEN, 'v', (GDK_CONTROL_MASK), NULL 
  },
  { GNOME_APP_UI_ITEM,
    N_("Edit"), 
    N_("Edit account information."), 
    gnc_ui_mainWindow_toolbar_edit, 
    NULL,
    NULL,
    GNOME_APP_PIXMAP_STOCK,
    GNOME_STOCK_PIXMAP_PROPERTIES, 'e', (GDK_CONTROL_MASK), NULL
  },
  GNOMEUIINFO_SEPARATOR,
  { GNOME_APP_UI_ITEM,
    N_("Add"),
    N_("Add a new account."),
    gnc_ui_add_account, 
    NULL,
    NULL,
    GNOME_APP_PIXMAP_STOCK,
    GNOME_STOCK_PIXMAP_ADD, 'a', (GDK_CONTROL_MASK), NULL
  },
  { GNOME_APP_UI_ITEM,
    N_("Remove"), 
    N_("Remove selected account."), 
    gnc_ui_delete_account_cb, 
    NULL,
    NULL,
    GNOME_APP_PIXMAP_STOCK,
    GNOME_STOCK_PIXMAP_REMOVE, 'r', (GDK_CONTROL_MASK), NULL
  },
  GNOMEUIINFO_SEPARATOR,
  { GNOME_APP_UI_ITEM,
    N_("Exit"), 
    N_("Exit GnuCash."),
    gnc_shutdown, 
    NULL,
    NULL,
    GNOME_APP_PIXMAP_STOCK,
    GNOME_STOCK_PIXMAP_QUIT, 'q', (GDK_CONTROL_MASK), NULL
  },
  GNOMEUIINFO_END
};

static gint
acct_ctree_select(GtkWidget *widget, GtkCTreeNode *row, gint column) 
{
  Account *account;
  
  account = (Account *)gtk_ctree_node_get_row_data(GTK_CTREE(widget), GTK_CTREE_NODE(row));
  gtk_object_set_data(GTK_OBJECT(app), "selected_account", account);
  
  return TRUE;
}

static gint
acct_ctree_unselect(GtkWidget *widget, GtkCTreeNode *row, gint column)
{
  gtk_object_set_data(GTK_OBJECT(app), "selected_account", NULL);
  
  return TRUE; 
}

Session *
gnc_main_window_get_session(gncUIWidget w) {
  /* FIXME: right now there's only one session.  Eventually we might
     allow multiple windows open. */
  return(current_session);
}

static void
gnc_ui_refresh_statusbar()
{
  GtkWidget *statusbar;
  guint     context_id;
  int i;
  double  assets  = 0.0;
  double  profits = 0.0;
  char buf[BUFSIZE];
  char *amt;
  AccountGroup *grp;
  Account *acc;
  int nacc;
   
  grp = xaccSessionGetGroup (current_session);
  //if (!grp) grp = topgroup;
  nacc = xaccGroupGetNumAccounts (grp);
  for (i=0; i<nacc; i++) {
     int acc_type;
     AccountGroup *acc_children;

     acc = xaccGroupGetAccount (grp,i);
 
     acc_type = xaccAccountGetType (acc);
     acc_children = xaccAccountGetChildren (acc);

     switch (acc_type) {
        case BANK:
        case CASH:
        case ASSET:
        case STOCK:
        case MUTUAL:
        case CREDIT:
        case LIABILITY:
           assets += xaccAccountGetBalance (acc);
           if (acc_children) {
              assets += xaccGroupGetBalance (acc_children);
           }
           break;
        case INCOME:
        case EXPENSE:
           profits -= xaccAccountGetBalance (acc); /* flip the sign !! */
           if (acc_children) {
              profits -= xaccGroupGetBalance (acc_children); /* flip the sign !! */
           }
           break;
        case EQUITY:
        default:
           break;
     }
  }
  
  amt = xaccPrintAmount (assets, PRTSYM | PRTSEP);
  strcpy (buf, " [ Assets: ");
  strcat (buf, amt);
  strcat (buf, " ] [ Profits: ");
  amt = xaccPrintAmount (profits, PRTSYM | PRTSEP);
  strcat (buf, amt);
  strcat (buf, "]");
   
  statusbar = gtk_object_get_data(GTK_OBJECT(app), "statusbar");

  context_id = gtk_statusbar_get_context_id( GTK_STATUSBAR(statusbar), 
                                            "Statusbar");
  
  gtk_statusbar_push( GTK_STATUSBAR(statusbar), context_id, buf);
  
}

/* Required for compatibility with Motif code... */
void
refreshMainWindow()
{
  gnc_ui_refresh_statusbar();
  gnc_ui_refresh_tree();
}

void
gnc_ui_acct_ctree_fill(GtkCTree *ctree, GtkCTreeNode *parent, AccountGroup *accts)
{
  GtkCTreeNode *sibling;
  gint          totalAccounts = xaccGroupGetNumAccounts(accts);
  gint          currentAccount;
  gchar        *text[3];
  
  sibling = NULL;
  
  /* Add each account to the tree */  
  for ( currentAccount = 0;
        currentAccount < totalAccounts;
        currentAccount++ )
  {
    Account      *acc = xaccGroupGetAccount(accts, currentAccount);
    AccountGroup *hasChildren;
    gchar         buf[BUFSIZE];
    GtkWidget    *popup;
        
    sprintf(buf, "%s%.2f", CURRENCY_SYMBOL, xaccAccountGetBalance(acc));
    
    text[0] = xaccAccountGetName(acc);
    text[1] = xaccAccountGetDescription(acc);
    text[2] = buf;
    
    sibling = gtk_ctree_insert_node (ctree, parent, sibling, text, 0,
                                     NULL, NULL, NULL, NULL,
				     FALSE, FALSE);
				           
    /* Set the user_data for the tree item to the account it */
    /* represents.                                           */
    gtk_ctree_node_set_row_data(GTK_CTREE(ctree), sibling, acc);
    
    popup = gnome_popup_menu_new(accountsmenu);
    gnome_popup_menu_attach (GTK_WIDGET(popup), GTK_WIDGET(ctree), NULL);

//    gtk_ctree_toggle_expansion(GTK_CTREE(ctree), GTK_CTREE_NODE(sibling));

    /* Connect the signal to the tree_select_row event */
    gtk_signal_connect (GTK_OBJECT(ctree), 
                        "tree_select_row",
                        GTK_SIGNAL_FUNC(acct_ctree_select), 
                        NULL);

    gtk_signal_connect (GTK_OBJECT(ctree), 
                        "tree_unselect_row",
                        GTK_SIGNAL_FUNC(acct_ctree_unselect), 
                        NULL);                        
    
    /* Check to see if this account has any children. If it
     * does then we need to build a subtree and fill it 
     */
    hasChildren = xaccAccountGetChildren(acc); 
     
    if(hasChildren)
    {
      /* Call gnc_ui_accWindow_tree_fill to fill this new subtree */
      gnc_ui_acct_ctree_fill(ctree, sibling, hasChildren );  
    }
  }
  				       
}

/********************************************************************\
 * refresh_tree                                                     *
 *   refreshes the main window                                      *
 *                                                                  *
 * Args:    tree - the tree that will get destroyed..               *
 * Returns: nothing                                                 *
\********************************************************************/
void
gnc_ui_refresh_tree() 
{
  GtkCTree     *ctree;
  GtkCTreeNode *parent;
  AccountGroup *accts;
  
  parent = gtk_object_get_data(GTK_OBJECT(app), "ctree_parent");
  ctree  = gtk_object_get_data(GTK_OBJECT(app), "ctree");
  
  accts  = xaccSessionGetGroup(current_session);
  
  gtk_ctree_remove_node(ctree, parent);

  free(parent);
  
  parent = NULL;
  
  gtk_clist_freeze(GTK_CLIST(ctree));
  gnc_ui_acct_ctree_fill(ctree, parent, accts);
  gtk_clist_thaw(GTK_CLIST(ctree));
  gtk_clist_columns_autosize(GTK_CLIST(ctree));  
 
}

static void
gnc_ui_about_cb (GtkWidget *widget, gpointer data)
{
  helpWindow( GTK_WIDGET(app), ABOUT_STR, HH_ABOUT ); 
}                          

static void
gnc_ui_help_cb ( GtkWidget *widget, gpointer data )
{
  helpWindow( GTK_WIDGET(app), HELP_STR, HH_MAIN );
}

static void
gnc_ui_reports_cb(GtkWidget *widget, gchar *report)
{
  reportWindow (widget, "duuuude", report);  
}

static void
gnc_ui_add_account ( GtkWidget *widget, gpointer data )
{
  GtkWidget *toplevel;
  Account   *account;
  
  toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget));
  account  = gtk_object_get_data(GTK_OBJECT(app), "selected_account");

  /* FIXME: Right now this really is not doing anything        */
  /*        The new account dialog should use this information */  
  /*        to set the parent account...                       */
  if (account)
  {
    accWindow((AccountGroup *)account);
  }
  else
  {
    accWindow(NULL);
  }
  
}

static void
gnc_ui_delete_account_finish_cb ( GtkWidget *widget, gpointer data )
{
  GtkCTreeNode *deleteRow;
  GtkCTreeNode *parentRow;
  GtkCTree     *ctree;
  Account      *account = data;

  ctree      = gtk_object_get_data(GTK_OBJECT(app), "ctree");
  parentRow  = gtk_object_get_data(GTK_OBJECT(app), "ctree_parent");

  /* Step 1: Delete the actual account */  
  xaccRemoveAccount ( account );
  xaccFreeAccount ( account );

  /* Step 2: Find the GtkCTreeNode that matches this account */
  deleteRow = gtk_ctree_find_by_row_data(GTK_CTREE(ctree), parentRow, account);
  
  /* Step 3: Delete the GtkCTreeNode we just found */
  gtk_ctree_remove_node(GTK_CTREE(ctree), deleteRow);
  
  /* Step 4: Refresh the toolbar */
  gnc_ui_refresh_statusbar();
}

static void
gnc_ui_delete_account_cb ( GtkWidget *widget, gpointer data )
{
  GtkWidget *toplevel;
  Account   *account;
  
  toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget));
  account  = gtk_object_get_data(GTK_OBJECT(app), "selected_account");
  

  if (account)
  {
    GtkWidget *msgbox;
      
     msgbox = gnome_message_box_new ( " Are you sure you want to delete this account. ",
                                       GNOME_MESSAGE_BOX_WARNING, 
                                       GNOME_STOCK_BUTTON_OK,
                                       GNOME_STOCK_BUTTON_CANCEL, NULL );
     gnome_dialog_button_connect (GNOME_DIALOG (msgbox), 0,
                                  GTK_SIGNAL_FUNC (gnc_ui_delete_account_finish_cb), 
                                  account);
     gtk_widget_show ( msgbox );
  }
  else
  {
    GtkWidget *msgbox;
    msgbox = gnome_message_box_new(ACC_DEL_MSG, GNOME_MESSAGE_BOX_ERROR, "Ok", NULL);
    gtk_widget_show ( msgbox );    
  }
}

static void
gnc_ui_mainWindow_toolbar_open ( GtkWidget *widget, gpointer data )
{
  Account   *account;
  GtkWidget *toplevel;
  
  toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget));
  account  = gtk_object_get_data(GTK_OBJECT(app), "selected_account");
  
  if(account)
  {
   fprintf(stderr, "calling regWindowSimple(%p)\n", account);
   regWindowSimple ( account );
  }
  else
  {
    GtkWidget *msgbox;
    msgbox = gnome_message_box_new ( " You must select an account to open first. ",
                                     GNOME_MESSAGE_BOX_ERROR, "Ok", NULL );
    gtk_widget_show ( msgbox );    
  }  
}

static void
gnc_ui_mainWindow_toolbar_edit ( GtkWidget *widget, gpointer data )
{
  GtkWidget *toplevel;
  Account   *account;
  
  toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget));
  account  = gtk_object_get_data(GTK_OBJECT(app), "selected_account");
  
  if (account)
  {
    editAccWindow( account );
  }
  else
  {
    GtkWidget *msgbox;
    msgbox = gnome_message_box_new(ACC_EDIT_MSG, GNOME_MESSAGE_BOX_ERROR, "Ok", NULL);
    gtk_widget_show ( msgbox );    
  }  
}

static void
gnc_ui_options_cb ( GtkWidget *widget, gpointer data ) {
  gnc_show_options_dialog();
}

void
mainWindow() {
  GtkWidget    *scrolled_win;
  GtkWidget    *main_vbox;
  GtkWidget    *statusbar;
  GtkWidget    *ctree;
  GtkCTreeNode *parent = NULL;
  AccountGroup *accts = xaccSessionGetGroup(current_session);
  gchar        *ctitles[] = {ACC_NAME_STR, DESC_STR, BALN_STR};

  /* Create ctree */
  ctree = gtk_ctree_new_with_titles(3, 0, ctitles);

  gtk_object_set_data(GTK_OBJECT(app), "ctree", ctree);
  gtk_object_set_data(GTK_OBJECT(app), "ctree_parent", parent);

  gnome_app_create_toolbar(GNOME_APP(app), toolbar);
  gnome_app_create_menus  (GNOME_APP(app), mainmenu);

  /* Cram accounts into the ctree widget & make sure the columns are sized correctly */
  gtk_clist_freeze(GTK_CLIST(ctree));
  gnc_ui_acct_ctree_fill(GTK_CTREE(ctree), parent, accts);
  gtk_clist_thaw(GTK_CLIST(ctree));

  gtk_clist_columns_autosize(GTK_CLIST(ctree));
  gtk_clist_set_shadow_type (GTK_CLIST(ctree), GTK_SHADOW_IN);

  /* Create main vbox */
  main_vbox = gtk_vbox_new( FALSE, 0 );
  gnome_app_set_contents ( GNOME_APP ( app ), main_vbox );

  /* create scrolled window */
  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                  GTK_POLICY_AUTOMATIC, 
                                  GTK_POLICY_AUTOMATIC);
  gtk_widget_show (scrolled_win);

  gtk_container_add(GTK_CONTAINER(scrolled_win), GTK_WIDGET(ctree));
  gtk_box_pack_start(GTK_BOX(main_vbox), scrolled_win, TRUE, TRUE, 0);

  /* create statusbar and pack it into the main_vbox */
  statusbar = gtk_statusbar_new ();
  gtk_object_set_data (GTK_OBJECT (app), "statusbar", statusbar);
  gtk_widget_show (statusbar);
  gtk_box_pack_start (GTK_BOX (main_vbox), statusbar, FALSE, FALSE, 0); 

  gtk_widget_set_usize ( GTK_WIDGET(app), 500, 400 );		      

  {
    SCM run_danglers = gh_eval_str("gnc:hook-run-danglers");
    SCM hook = gh_eval_str("gnc:*main-window-opened-hook*");
    SCM window = POINTER_TOKEN_to_SCM(make_POINTER_TOKEN("gncUIWidget", app));
    gh_call2(run_danglers, hook, window); 
  }
  
  /* Show everything now that it is created */

  gtk_widget_show(main_vbox);
  gtk_widget_show(ctree);
  gtk_widget_show(app);

  refreshMainWindow();

} 

/* OLD_GNOME_CODE */

void
gnc_ui_shutdown (GtkWidget *widget, gpointer *data) 
{
  gtk_main_quit ();
}
    
/********************* END OF FILE **********************************/

/*
  Local Variables:
  tab-width: 2
  indent-tabs-mode: nil
  mode: c-mode
  c-indentation-style: gnu
  eval: (c-set-offset 'block-open '-)
  End:
*/