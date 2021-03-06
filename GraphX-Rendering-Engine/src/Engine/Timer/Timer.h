#pragma once

namespace GraphX
{
	class Timer
	{
	public:
		/* Constructor */
		explicit Timer(const char* name);

		/* Update the timer */
		void Update();

		/* Returns the time between two successive updates */
		float GetDeltaTime() const;

		/* Returns the total time elapsed since the start of the Timer */
		float GetTime() const;

		/* Destructor for the timer */
		~Timer();

	private:
		/* Name of the timer */
		const char* m_Name;

		/* To store the time point of the last update */
		std::chrono::time_point<std::chrono::steady_clock> m_LastUpdateTimePoint;

		/* To store the time point when the application started */
		std::chrono::time_point<std::chrono::steady_clock> m_StartTimePoint;
	};
}