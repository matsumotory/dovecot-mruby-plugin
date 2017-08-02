/*
 * =====================================================================================
 *
 *       Filename:  imap-mruby-plugin-init.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/02/2017 09:26:35 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "mruby.h"
#include "mruby/compile.h"

#include "imap-mruby-plugin-imap.h"
#include "imap-mruby-plugin-init.h"

#define GC_ARENA_RESTORE mrb_gc_arena_restore(mrb, 0);

void imap_mruby_class_init(mrb_state *mrb)
{ 
  struct RClass *class;

  class = mrb_define_class(mrb, "Dovecot", mrb->object_class);

  imap_mruby_imap_class_init(mrb, class);
  GC_ARENA_RESTORE;
}
