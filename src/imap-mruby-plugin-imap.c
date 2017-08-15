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
#include "mruby/variable.h"
#include "mruby/hash.h"

static mrb_value imap_mruby_imap_cmd_name(mrb_state *mrb, mrb_value self)
{
  imap_mruby_internal_context *mctx;

  mctx = mrb->ud;

  return mrb_str_new_cstr(mrb, mctx->cmd->name);
}

static mrb_value imap_mruby_imap_username(mrb_state *mrb, mrb_value self)
{
  imap_mruby_internal_context *mctx;

  mctx = mrb->ud;

  return mrb_str_new_cstr(mrb, mctx->cmd->client->user->username);
}

static mrb_value imap_mruby_imap_session_id(mrb_state *mrb, mrb_value self)
{
  imap_mruby_internal_context *mctx;

  mctx = mrb->ud;

  return mrb_str_new_cstr(mrb, mctx->cmd->client->user->session_id);
}

static mrb_value imap_mruby_imap_command_register(mrb_state *mrb, mrb_value self)
{
  struct RClass *klass;
  mrb_sym cmd_hash_sym;
  mrb_value cmd_hash, cmd_name, cmd_block;

  mrb_get_args(mrb, "o&", &cmd_name, &cmd_block);

  command_register(mrb_str_to_cstr(mrb, cmd_name), cmd_mruby_handler, 0);

  cmd_hash_sym = mrb_intern_lit(mrb, IMAP_MRUBY_COMMAND_REG_ID);
  klass = mrb_class_get_under(mrb, mrb_class_get(mrb, "Dovecot"), "IMAP");
  cmd_hash = mrb_mod_cv_get(mrb, klass, cmd_hash_sym);

  if (mrb_nil_p(cmd_hash)) {
    cmd_hash = mrb_hash_new(mrb);
  }

  cmd_name = mrb_funcall(mrb, cmd_name, "upcase", 0, NULL);
  mrb_hash_set(mrb, cmd_hash, cmd_name, cmd_block);
  mrb_mod_cv_set(mrb, klass, cmd_hash_sym, cmd_hash);

  return cmd_hash;
}

void imap_mruby_imap_class_init(mrb_state *mrb, struct RClass *class)
{
  struct RClass *class_imap4;

  class_imap4 = mrb_define_class_under(mrb, class, "IMAP", mrb->object_class);
  mrb_mod_cv_set(mrb, class_imap4, mrb_intern_lit(mrb, IMAP_MRUBY_COMMAND_REG_ID), mrb_nil_value());
  mrb_define_class_method(mrb, class_imap4, "command_name", imap_mruby_imap_cmd_name, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, class_imap4, "username", imap_mruby_imap_username, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, class_imap4, "session_id", imap_mruby_imap_session_id, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, class_imap4, "command_register", imap_mruby_imap_command_register,
                          MRB_ARGS_REQ(1) | MRB_ARGS_BLOCK());
}
