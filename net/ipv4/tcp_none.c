/* Specifically does not do congestion control
 *
 * Bruce Spang <bruce@brucespang.com>
 */

#include <linux/module.h>
#include <net/tcp.h>

// TODO: tried to use TCP_INFINITE_SSTHRESH here, but it's too large and
// somehow this causes problems. This seems fine for me, would like something a little
// more generic though.
#define MAX_CWND 10000

static void tcp_none_cong_control(struct sock *sk, const struct rate_sample *rs)
{
  struct tcp_sock *tp = tcp_sk(sk);
  tp->snd_cwnd = MAX_CWND;
  tp->snd_ssthresh = MAX_CWND;
  sk->sk_pacing_rate = sk->sk_max_pacing_rate;
}

static u32 tcp_none_ssthresh(struct sock *sk)
{
	return MAX_CWND;
}

static u32 tcp_none_undo_cwnd(struct sock *sk)
{
  return tcp_sk(sk)->snd_cwnd;
}

static void tcp_none_init(struct sock *sk)
{
  struct tcp_sock *tp = tcp_sk(sk);
  tp->snd_ssthresh = MAX_CWND;
  tp->snd_cwnd = MAX_CWND;
  sk->sk_pacing_rate = sk->sk_max_pacing_rate;
  
  cmpxchg(&sk->sk_pacing_status, SK_PACING_NONE, SK_PACING_NEEDED);
}

static struct tcp_congestion_ops tcp_none __read_mostly = {
  .init       = tcp_none_init, 
	.ssthresh	  = tcp_none_ssthresh,
	.undo_cwnd	= tcp_none_undo_cwnd,
	.cong_control	= tcp_none_cong_control,

	.owner		= THIS_MODULE,
	.name		= "none",
};

static int __init tcp_none_register(void)
{
	return tcp_register_congestion_control(&tcp_none);
}

static void __exit tcp_none_unregister(void)
{
	tcp_unregister_congestion_control(&tcp_none);
}

module_init(tcp_none_register);
module_exit(tcp_none_unregister);

MODULE_AUTHOR("Bruce Spang <bruce@brucespang.com>");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("TCP None (No congestion control)");
