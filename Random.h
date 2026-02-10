/*
	Random.h
	20250914 hanaue sho
	ランダム用クラス
	ほぼChatGPT君に作って貰いました
*/
#ifndef RANDOM_H_
#define RANDOM_H_
#include <random>	 // 乱数エンジン、分布
#include <cstdint>	 // 固定幅整数
#include <algorithm> // std::swap
#include <limits>	 // 数値限界
#include <cmath>	 // std::nextafter
#include <chrono>	 // 時刻でシードを混ぜる

class Random 
{
public:
	using engine_t = std::mt19937; // メルセンヌツイスター32bit（軽快＆高品質）

private:
	static engine_t& Global()
	{
		thread_local engine_t eng{ SeedFromDevice() };
		return eng;
	}
	// 初期シードを作る（random_device に時刻やアドレス値を混ぜてエントロピー確保） 
	static engine_t::result_type SeedFromDevice()
	{
		std::random_device rd;
		// rd が疑似的な環境でも。時間とアドレスを混ぜてある程度ランダムに
		auto now = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		auto mix = (static_cast<uint64_t>(rd()) << 32) ^ rd() ^ now ^ reinterpret_cast<uintptr_t>(&now); // rd() と時刻、スタック上のアドレスを XOR 混合
		return static_cast<engine_t::result_type>(mix); // mt19937 のシード型に丸めて返す（32bit）
	}

public:
	// 現在のスレッドのRNGを任意シードで初期化（シード値を固定したい時用）
	static void Seed(uint64_t seed)
	{
		// Global() は thread_local なエンジンを参照で返す
		Global() = engine_t(static_cast<engine_t::result_type>(seed));
	}

	static float RandomRange(float min, float max)
	{
		if (min > max) std::swap(min, max);
		std::uniform_real_distribution<float> dist(min, 
												   std::nextafter(max, std::numeric_limits<float>::infinity()));
		return dist(Global()); // 分布にエンジンを食わせて値を一つ生成
	}
	// [min, max]
	static int RandomRange(int min, int max) 
	{
		if (min > max) std::swap(min, max);
		std::uniform_int_distribution<int> dist(min, max); // こちらは規格上 [a,b]
		return dist(Global());
	}
	// [0,1) の実数（シンプル版）
	static float Random01()
	{
		std::uniform_real_distribution<float> dist(0.0f, 1.0f); // [0,1)
		return dist(Global());
	}

	// 離散一様: min, min+step, min+2*step, ... <= max のいずれかを等確率で返す
	static float RandomRangeStepped(float min, float max, float step)
	{
		// step が正かつ有限であることを確認
		if (!(step > 0.0f) || !std::isfinite(step)) return std::numeric_limits<float>::quiet_NaN();
		if (min > max) std::swap(min, max);                        // 引数が逆でもOKに

		// 計算は誤差を抑えるため double で行う
		const double dmin = static_cast<double>(min);
		const double dmax = static_cast<double>(max);
		const double dstep = static_cast<double>(step);

		// グリッド上の「最大インデックス」k_max を求める（min + k*step <= max）
		// 1e-12 は浮動小数による僅かな超過を吸収するための微小量
		const double kmax_d = std::floor((dmax - dmin) / dstep + 1e-12);
		const int    kmax = (kmax_d < 0.0) ? 0 : static_cast<int>(kmax_d);

		// k ∈ [0, kmax] を一様に選ぶ（両端含む）
		const int k = RandomRange(0, kmax);

		// 値を構成（最後に float へ）。fmaf で丸め誤差を一段抑えるのもアリ
		return static_cast<float>(dmin + dstep * static_cast<double>(k));
	}
};

#endif