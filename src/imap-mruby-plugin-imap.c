/*
 * =====================================================================================
 *
 *       Filename:  imap-mruby-plugin-imap.c
 *
 *    Description:  Dovecot::IMAP4 class
 *
 *        Version:  1.0
 *        Created:  08/02/2017 09:06:25 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ryosuke Matsumoto, 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "imap-mruby-plugin.h"
#include "imap-mruby-plugin-imap.h"

#include "mruby.h"
#include "mruby/class.h"
#include "mruby/compile.h"
#include "mruby/data.h"
#include "mruby/proc.h"
#include "mruby/string.h"

static mrb_value imap_mruby_imap_cmd_name(mrb_state *mrb, mrb_value self)
{
  imap_mruby_internal_context *mctx = mrb->ud; 
  return mrb_str_new_cstr(mrb, mctx->cmd->name);
}

void imap_mruby_imap_class_init(mrb_state *mrb, struct RClass *class)
{
  struct RClass *class_imap4;

  class_imap4 = mrb_define_class_under(mrb, class, "IMAP4", mrb->object_class);
  mrb_define_class_method(mrb, class_imap4, "command_name", imap_mruby_imap_cmd_name, MRB_ARGS_NONE());
}

