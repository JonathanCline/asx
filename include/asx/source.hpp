#pragma once

/**
 * @file Provides utilities for describing source code info (such as location).
*/

#include <asx/assert.hpp>

#include <span>
#include <array>
#include <cstdint>
#include <algorithm>
#include <string_view>
#include <source_location>

namespace asx
{
	/**
	 * @brief The max number of stack frames to use for the stack trace type by default.
	*/
	constexpr inline size_t STACK_TRACE_MAX_FRAMES_DEFAULT = 32;

	/**
	 * @brief The maximum size of the file name string in characters.
	*/
	constexpr inline size_t SOURCE_LOCATION_FILE_NAME_MAX = 128;

	/**
	 * @brief The maximum size of the function name string in characters.
	*/
	constexpr inline size_t SOURCE_LOCATION_FUNCTION_NAME_MAX = 96;
};



/**
 * @brief Macro alias that evaluates to the current file name as a string literal.
*/
#define ASX_FILE __FILE__

/**
 * @brief Macro alias that evaluates to the current line number as an integer.
*/
#define ASX_LINE __LINE__

/**
 * @brief Macro alias that evaluates to the current scope's function name as a string literal.
*/
#define ASX_FUNCTION __FUNCTION__



namespace asx
{
	/**
	 * @brief Removes the machine-specific part of an astrum source file's path.
	 *
	 * This is used to make source file paths only contain where the file exists within the project,
	 * rather than on the actual machine.
	 *
	 * @return The section of the given path after the project source path, or `_path` if the project source root string wasn't found.
	*/
	constexpr std::string_view fix_project_source_file_path(std::string_view _path)
	{
		if (_path.empty()) { return _path; };

		const auto _pathBegin = _path.begin();
		const auto _pathEnd = _path.end();

		constexpr auto _sourceRoot = std::string_view(ASX_PROJECT_SOURCE_ROOT);
		constexpr auto _sourceRootBegin = _sourceRoot.begin();
		constexpr auto _sourceRootEnd = _sourceRoot.end();

		auto _pathIter = _pathBegin;
		for (auto _sourceRootIter = _sourceRootBegin; ; ++_sourceRootIter, ++_pathIter)
		{
			// Handle end of `_astrumSourceRoot` string
			if (_sourceRootIter == _sourceRootEnd)
			{
				if (_pathIter == _pathEnd)
				{
					// Not a path INTO the project source code, return original `_path` pointer
					return _path;
				}
				else
				{
					// Path into the project source code was found, exit loop
					break;
				};
			}
			// Handle end of given `_path`
			else if (_pathIter == _pathEnd)
			{
				// Not a path into the project source code, return original `_path` pointer
				return _path;
			}
			// Extra handling for directory seperators
			else if (*_sourceRootIter == '/' || *_sourceRootIter == '\\')
			{
				if (*_pathIter != '/' && *_pathIter != '\\')
				{
					// Mismatch found, return original `_path` pointer
					return _path;
				};

				// Continue looping
			}
			else if (*_sourceRootIter != *_pathIter)
			{
				// Mismatch found, return original `_path` pointer
				return _path;
			};
		};

		// `_pathIter` should now point to the start of the relative path
		return std::string_view(_pathIter, _pathEnd);
	};

	/**
	 * @brief Describes a location within the project source code.
	*/
	class SourceLocation
	{
	public:

		/**
		 * @brief Gets the max size of the file name string in characters (does not include null-terminator as the stored string doesn't use one).
		 * @return Size in characters.
		*/
		constexpr static size_t file_name_max_size()
		{
			return asx::SOURCE_LOCATION_FILE_NAME_MAX;
		};

		/**
		 * @brief Gets the max size of the function name string in characters (does not include null-terminator as the stored string doesn't use one).
		 * @return Size in characters.
		*/
		constexpr static size_t function_name_max_size()
		{
			return asx::SOURCE_LOCATION_FUNCTION_NAME_MAX;
		};

		/**
		 * @brief Gets the name of the source code file.
		 * @return File name string.
		*/
		constexpr std::string_view file() const
		{
			return std::string_view
			(
				this->file_.begin(),
				std::ranges::find(this->file_, '\0')
			);
		};

		/**
		 * @brief Gets the name of the source code function.
		 * @return Function name string.
		*/
		constexpr std::string_view function() const
		{
			return std::string_view
			(
				this->function_.begin(),
				std::ranges::find(this->function_, '\0')
			);
		};

		/**
		 * @brief Gets the source code line number.
		 * @return File line number.
		*/
		constexpr uint32_t line() const
		{
			return this->line_;
		};

		/**
		 * @brief Constructs a source location using an absolute path.
		 * 
		 * This function will remove the project source code root path from the given file path if it
		 is present. Otherwise, this is identical to the regular `SourceLocation` constructor.
		 *
		 * @param _file Absolute path to the source file.
		 * @param _function Name of the function.
		 * @param _line Line number within the source code file.
		*/
		constexpr static SourceLocation from_absolute_path(std::string_view _file, std::string_view _function, uint32_t _line)
		{
			return SourceLocation(asx::fix_project_source_file_path(_file), _function, _line);
		};

		constexpr SourceLocation() :
			file_{}, function_{}, line_(0)
		{};
		constexpr SourceLocation(std::string_view _file, std::string_view _function, uint32_t _line) :
			file_{}, function_{}, line_(_line)
		{
			// Copy file name.
			std::copy(_file.begin(), _file.begin() + (std::min(_file.size(), this->file_.size())), this->file_.begin());

			// Copy function name.
			std::copy(_function.begin(), _function.begin() + (std::min(_function.size(), this->function_.size())), this->function_.begin());
		};


	private:
		std::array<char, SOURCE_LOCATION_FILE_NAME_MAX> file_;
		std::array<char, SOURCE_LOCATION_FUNCTION_NAME_MAX> function_;
		uint32_t line_;
	};
};

/**
 * @brief Makes an `asx::SourceLocation` object using the current line/function/file info.
*/
#define ASX_HERE() ::asx::SourceLocation::from_absolute_path(ASX_FILE, ASX_FUNCTION, ASX_LINE)


// Stack tracing

namespace asx
{
	/**
	 * @brief Gets a stack trace if possible.
	 * @param _outBuffer Buffer of stack frame objects to write stack frames into.
	 * @param _skipFrames The number of stack frames to skip, defaults to 0 which starts in the calling function.
	 * @return Number of frames written.
	*/
	size_t get_stack_trace(std::span<SourceLocation> _outBuffer, size_t _skipFrames = 0);

	/**
	 * @brief Holds a number of stack frames composing a stack trace.
	 * 
	 * This does NOT allocate. Be aware that the size tends to get quite large as this holds
	 * an array of the frames internally. Modify the `MaxFrames` tparam as needed to help minimize
	 * the size problem.
	 * 
	 * @tparam MaxFrames The maximum number of stack frames this can store.
	*/
	template <size_t MaxFrames>
	struct BasicStackTrace
	{
	public:

		/**
		 * @brief The type stored in this container.
		*/
		using value_type = SourceLocation;

	private:

		/**
		 * @brief The container type for storing the frames.
		*/
		using container_type = std::array<value_type, MaxFrames>;

	public:

		using iterator = typename container_type::iterator;
		using const_iterator = typename container_type::const_iterator;

		/**
		 * @brief Gets the max number of frames this stack trace may contain.
		 * @return Size in stack frames.
		*/
		constexpr size_t max_size() const noexcept
		{
			return MaxFrames;
		};

		/**
		 * @brief Gets the number of frames stored in this stack trace.
		 * @return Size in stack frames.
		*/
		constexpr size_t size() const noexcept
		{
			return this->count_;
		};
		
		constexpr iterator begin() noexcept
		{
			return this->frames_.begin();
		};
		constexpr const_iterator begin() const noexcept
		{
			return this->frames_.cbegin();
		};
		constexpr const_iterator cbegin() const noexcept
		{
			return this->frames_.cbegin();
		};

		constexpr iterator end()
		{
			return this->begin() + this->size();
		};
		constexpr const_iterator end() const
		{ 
			return this->cbegin() + this->size();
		};
		constexpr const_iterator cend() const noexcept
		{
			return this->cbegin() + this->size();
		};

		/**
		 * @brief Resizes the used portion of the stack frame buffer.
		 * 
		 * This doesn't actually change the amount of memory this type uses.
		 * The only thing that will change is where the end iterator points to.
		 * 
		 * @param _newSize The new size in stack frames, must be less than or equal to `MaxFrames` (`max_size()`).
		*/
		constexpr void resize(size_t _newSize)
		{
			ASX_ASSERT(_newSize <= this->max_size());
			const auto _oldSize = this->size();

			// If the container is shrinking, "clear" the now out of bounds frames.
			if (_newSize < _oldSize)
			{
				for (auto it = this->frames_.begin() + _newSize; it != this->frames_.begin() + _oldSize; ++it)
				{
					*it = value_type();
				};
			};

			// Store the new size.
			this->count_ = _newSize;
		};

		constexpr BasicStackTrace() = default;

		/**
		 * @brief Presizes the stack trace frame storage.
		 * 
		 * Note: this does NOT allocate. Effects are the same as calling `resize()`.
		 * 
		 * @param _size The size The new size in stack frames, must be less than or equal to `MaxFrames` (`max_size()`).
		*/
		constexpr explicit BasicStackTrace(size_t _size) :
			frames_{},
			count_(_size)
		{
			ASX_ASSERT(_size <= this->max_size());
		};

	private:
		container_type frames_;
		size_t count_;
	};

	/**
	 * @brief View type allowing immutable access to a span of stack frames.
	*/
	using StackTraceView = std::span<const asx::SourceLocation>;

	/**
	 * @brief Gets a stack trace if possible.
	 * @tparam MaxFrames The maximum number of stack frames to return, defaults to `DEFAULT_STACK_TRACE_MAX_FRAMES`.
	 * @param _skipFrames The number of stack frames to skip, defaults to 0 which starts in the calling function.
	 * @return The stack frames in the traceback.
	*/
	template <size_t MaxFrames = asx::STACK_TRACE_MAX_FRAMES_DEFAULT>
	inline BasicStackTrace<MaxFrames> get_stack_trace(size_t _skipFrames = 0)
	{
		auto _trace = BasicStackTrace<MaxFrames>(MaxFrames);
		auto _frames = std::span<SourceLocation>(_trace);
		const auto _actualFrameCount = get_stack_trace(_frames, _skipFrames + 1);
		_trace.resize(_actualFrameCount);
		return _trace;
	};

	/**
	 * @brief Holds a number of stack frames composing a stack trace.
	 *
	 * This does NOT allocate. Be aware that the size tends to get quite large as this holds
	 * an array of the frames internally. Modify the `MaxFrames` tparam as needed to help minimize
	 * the size problem.
	 *
	 * This is a type alias of `BasicStackTrace` using a reasonable default for the max frames template parameter.
	*/
	using StackTrace = BasicStackTrace<asx::STACK_TRACE_MAX_FRAMES_DEFAULT>;


};