/*******************************************************************************
 *
 * TNeoKernel: real-time kernel initially based on TNKernel
 *
 *    TNKernel:                  copyright � 2004, 2013 Yuri Tiomkin.
 *    PIC32-specific routines:   copyright � 2013, 2014 Anders Montonen.
 *    TNeoKernel:                copyright � 2014       Dmitry Frank.
 *
 *    TNeoKernel was born as a thorough review and re-implementation of
 *    TNKernel. The new kernel has well-formed code, inherited bugs are fixed
 *    as well as new features being added, and it is tested carefully with
 *    unit-tests.
 *
 *    API is changed somewhat, so it's not 100% compatible with TNKernel,
 *    hence the new name: TNeoKernel.
 *
 *    Permission to use, copy, modify, and distribute this software in source
 *    and binary forms and its documentation for any purpose and without fee
 *    is hereby granted, provided that the above copyright notice appear
 *    in all copies and that both that copyright notice and this permission
 *    notice appear in supporting documentation.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE DMITRY FRANK AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DMITRY FRANK OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *    THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/

//-- common tnkernel headers
#include "tn_common.h"
#include "tn_sys.h"
#include "tn_internal.h"

//-- header of current module
#include "tn_timer.h"

//-- header of other needed modules
#include "tn_tasks.h"





/*******************************************************************************
 *    PUBLIC DATA
 ******************************************************************************/

struct TN_ListItem tn_timer_list__gen;
struct TN_ListItem tn_timer_list__tick[ TN_TICK_LISTS_CNT ];



/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/

#define TN_TICK_LISTS_MASK    (TN_TICK_LISTS_CNT - 1)

//-- configuration check
#if ((TN_TICK_LISTS_MASK & TN_TICK_LISTS_CNT) != 0)
#  error TN_TICK_LISTS_CNT must be a power of two
#endif

#if (TN_TICK_LISTS_CNT < 2)
#  error TN_TICK_LISTS_CNT must be >= 2
#endif

/**
 * @param timeout    should be < TN_TICK_LISTS_CNT
 */
#define _TICK_LIST_INDEX(timeout)    \
   ((tn_sys_time_count + timeout) & TN_TICK_LISTS_MASK)




/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/

//-- Additional param checking {{{
#if TN_CHECK_PARAM
static inline enum TN_RCode _check_param_generic(
      struct TN_Timer *timer
      )
{
   enum TN_RCode rc = TN_RC_OK;

   if (timer == NULL){
      rc = TN_RC_WPARAM;
   } else if (timer->id_timer != TN_ID_TIMER){
      rc = TN_RC_INVALID_OBJ;
   }

   return rc;
}

static inline enum TN_RCode _check_param_create(
      struct TN_Timer  *timer,
      TN_TimerFunc     *func
      )
{
   enum TN_RCode rc = TN_RC_OK;

   if (timer == NULL){
      rc = TN_RC_WPARAM;
   } else if (timer->id_timer == TN_ID_TIMER){
      rc = TN_RC_WPARAM;
   }

   return rc;
}

#else
#  define _check_param_generic(timer)           (TN_RC_OK)
#  define _check_param_create(timer, func)      (TN_RC_OK)
#endif
// }}}



/*******************************************************************************
 *    PUBLIC FUNCTIONS
 ******************************************************************************/

/*
 * See comments in the header file (tn_timer.h)
 */
enum TN_RCode tn_timer_create(
      struct TN_Timer  *timer,
      TN_TimerFunc     *func,
      void             *p_user_data
      )
{
   enum TN_RCode rc = TN_RC_OK;

   rc = _check_param_create(timer, func);
   if (rc != TN_RC_OK){
      goto out;
   }

   rc = _tn_timer_create(timer, func, p_user_data);

out:
   return rc;
}

/*
 * See comments in the header file (tn_timer.h)
 */
enum TN_RCode tn_timer_delete(struct TN_Timer *timer)
{
   int sr_saved;
   enum TN_RCode rc = _check_param_generic(timer);

   if (rc == TN_RC_OK){
      sr_saved = tn_arch_sr_save_int_dis();
      //-- if timer is active, cancel it first
      rc = _tn_timer_cancel(timer);

      //-- now, delete timer
      timer->id_timer = 0;
      tn_arch_sr_restore(sr_saved);
   }

   return rc;
}

/*
 * See comments in the header file (tn_timer.h)
 */
enum TN_RCode tn_timer_start(struct TN_Timer *timer, TN_Timeout timeout)
{
   int sr_saved;
   enum TN_RCode rc = _check_param_generic(timer);

   if (rc == TN_RC_OK){
      sr_saved = tn_arch_sr_save_int_dis();
      rc = _tn_timer_start(timer, timeout);
      tn_arch_sr_restore(sr_saved);
   }

   return rc;
}

/*
 * See comments in the header file (tn_timer.h)
 */
enum TN_RCode tn_timer_cancel(struct TN_Timer *timer)
{
   int sr_saved;
   enum TN_RCode rc = _check_param_generic(timer);

   if (rc == TN_RC_OK){
      sr_saved = tn_arch_sr_save_int_dis();
      rc = _tn_timer_cancel(timer);
      tn_arch_sr_restore(sr_saved);
   }

   return rc;
}

/*
 * See comments in the header file (tn_timer.h)
 */
enum TN_RCode tn_timer_set_func(
      struct TN_Timer  *timer,
      TN_TimerFunc     *func,
      void             *p_user_data
      )
{
   int sr_saved;
   enum TN_RCode rc = _check_param_generic(timer);

   if (rc == TN_RC_OK){
      sr_saved = tn_arch_sr_save_int_dis();
      rc = _tn_timer_set_func(timer, func, p_user_data);
      tn_arch_sr_restore(sr_saved);
   }

   return rc;
}

/*
 * See comments in the header file (tn_timer.h)
 */
enum TN_RCode tn_timer_is_active(struct TN_Timer *timer, BOOL *p_is_active)
{
   int sr_saved;
   enum TN_RCode rc = _check_param_generic(timer);

   if (rc == TN_RC_OK){
      sr_saved = tn_arch_sr_save_int_dis();
      *p_is_active = _tn_timer_is_active(timer);
      tn_arch_sr_restore(sr_saved);
   }

   return rc;
}

/*
 * See comments in the header file (tn_timer.h)
 */
enum TN_RCode tn_timer_time_left(
      struct TN_Timer *timer,
      TN_Timeout *p_time_left
      )
{
   int sr_saved;
   enum TN_RCode rc = _check_param_generic(timer);

   if (rc == TN_RC_OK){
      sr_saved = tn_arch_sr_save_int_dis();
      *p_time_left = _tn_timer_time_left(timer);
      tn_arch_sr_restore(sr_saved);
   }

   return rc;
}





/*******************************************************************************
 *    PROTECTED FUNCTIONS
 ******************************************************************************/

/**
 * See comments in the tn_internal.h file,
 * see also \ref timers_implementation.
 */
void _tn_timers_tick_proceed(void)
{
#if TN_DEBUG
   //-- interrupts should be disabled here
   if (!TN_IS_INT_DISABLED()){
      _TN_FATAL_ERROR("");
   }
#endif

   int tick_list_index = _TICK_LIST_INDEX(0);

   if (tick_list_index == 0){
      //-- it happens each TN_TICK_LISTS_CNT-th system tick:
      //   now we should walk through all the timers in the "generic" timer
      //   list, decrement the timeout of each one by TN_TICK_LISTS_CNT,
      //   and if resulting timeout is less than TN_TICK_LISTS_CNT,
      //   then move that timer to the appropriate "tick" timer list.

      //-- handle "generic" timer list {{{
      {
         struct TN_Timer *timer;
         struct TN_Timer *tmp_timer;

         tn_list_for_each_entry_safe(
               timer, tmp_timer, &tn_timer_list__gen, timer_queue
               )
         {
#if TN_DEBUG
            if (     timer->timeout_cur == TN_WAIT_INFINITE 
                  || timer->timeout_cur < TN_TICK_LISTS_CNT
               )
            {
               //-- should never be here: timeout value should always
               //   be >= TN_TICK_LISTS_CNT here.
               //   And it should never be TN_WAIT_INFINITE.
               _TN_FATAL_ERROR();
            }
#endif
            timer->timeout_cur -= TN_TICK_LISTS_CNT;

            if (timer->timeout_cur < TN_TICK_LISTS_CNT){
               //-- it's time to move this timer to the "tick" list
               tn_list_remove_entry(&(timer->timer_queue));
               tn_list_add_tail(
                     &tn_timer_list__tick[_TICK_LIST_INDEX(timer->timeout_cur)],
                     &(timer->timer_queue)
                     );
            }
         }
      }
      //}}}
   }

   //-- it happens every system tick:
   //   we should walk through all the timers in the current "tick" timer list,
   //   and fire them all, unconditionally.

   //-- handle current "tick" timer list {{{
   {
      struct TN_Timer *timer;

      struct TN_ListItem *p_cur_timer_list = 
         &tn_timer_list__tick[ tick_list_index ];

      //-- now, p_cur_timer_list is a list of timers that we should
      //   fire NOW, unconditionally.

      //-- NOTE that we shouldn't use iterators like 
      //   `tn_list_for_each_entry_safe()` here, because timers can be 
      //   removed from the list while we are iterating through it: 
      //   this may happen if user-provided function cancels timer which 
      //   is in the same list.
      //
      //   Although timers could be removed from the list, note that
      //   new timer can't be added to it
      //   (because timeout 0 is disallowed, and timer with timeout
      //   TN_TICK_LISTS_CNT is added to the "generic" list),
      //   see implementation details in the tn_timer.h file
      while (!tn_is_list_empty(p_cur_timer_list)){
         timer = tn_list_first_entry(
               p_cur_timer_list, typeof(*timer), timer_queue
               );

         //-- first of all, cancel timer, so that 
         //   function could start it again if it wants to.
         _tn_timer_cancel(timer);

         //-- call user function
         timer->func(timer, timer->p_user_data);
      }

#if TN_DEBUG
      //-- current "tick" list should be empty now
      if (!tn_is_list_empty(p_cur_timer_list)){
         _TN_FATAL_ERROR("");
      }
#endif
   }
   // }}}
}

enum TN_RCode _tn_timer_start(struct TN_Timer *timer, TN_Timeout timeout)
{
   enum TN_RCode rc = TN_RC_OK;

#if TN_DEBUG
   //-- interrupts should be disabled here
   if (!TN_IS_INT_DISABLED()){
      _TN_FATAL_ERROR("");
   }
#endif

   if (timeout == TN_WAIT_INFINITE || timeout == 0){
      rc = TN_RC_WPARAM;
   } else {

      //-- if timer is active, cancel it first
      rc = _tn_timer_cancel(timer);

      if (timeout < TN_TICK_LISTS_CNT){
         //-- timer should be added to the one of "tick" lists.
         int tick_list_index = _TICK_LIST_INDEX(timeout);
         timer->timeout_cur = tick_list_index;

         tn_list_add_tail(
               &tn_timer_list__tick[ tick_list_index ],
               &(timer->timer_queue)
               );
      } else {
         //-- timer should be added to the "generic" list.
         //   We should set timeout_cur adding current "tick" index to it.
         timer->timeout_cur = timeout + _TICK_LIST_INDEX(0);

         tn_list_add_tail(&tn_timer_list__gen, &(timer->timer_queue));
      }
   }

   return rc;
}

enum TN_RCode _tn_timer_cancel(struct TN_Timer *timer)
{
   enum TN_RCode rc = TN_RC_OK;

#if TN_DEBUG
   //-- interrupts should be disabled here
   if (!TN_IS_INT_DISABLED()){
      _TN_FATAL_ERROR("");
   }
#endif

   if (_tn_timer_is_active(timer)){
      //-- reset timeout to zero (but this is actually not necessary)
      timer->timeout_cur = 0;

      //-- remove entry from timer queue
      tn_list_remove_entry(&(timer->timer_queue));

      //-- reset the list
      tn_list_reset(&(timer->timer_queue));
   }

   return rc;
}

enum TN_RCode _tn_timer_create(
      struct TN_Timer  *timer,
      TN_TimerFunc     *func,
      void             *p_user_data
      )
{
   enum TN_RCode rc = _tn_timer_set_func(timer, func, p_user_data);

   if (rc != TN_RC_OK){
      goto out;
   }

   tn_list_reset(&(timer->timer_queue));

   timer->timeout_cur   = 0;
   timer->id_timer      = TN_ID_TIMER;

out:
   return rc;
}

enum TN_RCode _tn_timer_set_func(
      struct TN_Timer  *timer,
      TN_TimerFunc     *func,
      void             *p_user_data
      )
{
   enum TN_RCode rc = TN_RC_OK;

   if (func == NULL){
      rc = TN_RC_WPARAM;
   } else {
      timer->func          = func;
      timer->p_user_data   = p_user_data;
   }

   return rc;
}

BOOL _tn_timer_is_active(struct TN_Timer *timer)
{
#if TN_DEBUG
   //-- interrupts should be disabled here
   if (!TN_IS_INT_DISABLED()){
      _TN_FATAL_ERROR("");
   }
#endif

   return (!tn_is_list_empty(&(timer->timer_queue)));
}

void _tn_timers_init(void)
{
   int i;

   //-- reset "generic" timers list
   tn_list_reset(&tn_timer_list__gen);

   //-- reset all "tick" timer lists
   for (i = 0; i < TN_TICK_LISTS_CNT; i++){
      tn_list_reset(&tn_timer_list__tick[i]);
   }
}

TN_Timeout _tn_timer_time_left(struct TN_Timer *timer)
{
#if TN_DEBUG
   //-- interrupts should be disabled here
   if (!TN_IS_INT_DISABLED()){
      _TN_FATAL_ERROR("");
   }
#endif

   TN_Timeout time_left = 0;

   if (_tn_timer_is_active(timer)){
      int tick_list_index = _TICK_LIST_INDEX(0);

      if (timer->timeout_cur > tick_list_index){
         time_left = timer->timeout_cur - tick_list_index;
      } else if (timer->timeout_cur < tick_list_index){
         time_left = timer->timeout_cur + TN_TICK_LISTS_CNT - tick_list_index;
      } else {
#if TN_DEBUG
         //-- timer->timeout_cur should never be equal to tick_list_index here
         //   (if it is, timer is inactive, so, we don't get here)
         _TN_FATAL_ERROR();
#endif
      }
   }

   return time_left;
}


