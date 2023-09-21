#pragma once

/**
 * @file 
 * @brief Provides command line argument parsing. Based on python's argparse module.
*/

#include <any>
#include <span>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace asx
{
	/**
	 * @brief Allows the creation of a parser with specified arguments and/or options.
	*/
	class ArgumentParser
	{
	private:

		/**
		 * @brief Data structure for defining an argument that can be parsed.
		*/
		struct ArgumentDefinition
		{
			/**
			 * @brief Optional label used to refer to the argument by a string.
			*/
			std::string label;

			/**
			 * @brief Names that can be used to refer to this argument.
			 * 
			 * An argument with names specified is assumed to be optional.
			*/
			std::vector<std::string> names;

			/**
			 * @brief Optional text used when naming this argument for the help text.
			 * 
			 * If this is not provided, the `names` will be used.
			 * If the argument doesn't have any names set then this will use the `label` text.
			 * If no `label` text has been set then this will use the argument's position.
			*/
			std::string metalabel;

			/**
			 * @brief Optional description text used for the help text.
			*/
			std::string description;

			/**
			 * @brief Set to true if argument doesn't have to be specified.
			*/
			bool is_optional = false;

			/**
			 * @brief Set to true if argument is positional. Positional arguments can be
			 * optional if all of the optional positional arguments are at the end.
			*/
			bool is_positional = false;
		};

	public:

		/**
		 * @brief Holds a parsed value, may be one of a handful of types as
		 * specified by the parser's arguments.
		*/
		class ParsedValue
		{
		public:

			/**
			 * @brief Exception thrown if type attempted to be accessed
			 * is not the actual stored type.
			*/
			using exception = std::bad_any_cast;

			/**
			 * @brief Checks if this value was provided.
			 *
			 * @return True if a value was provided, false otherwise.
			*/
			bool has_value() const noexcept
			{
				return this->value_.has_value();
			};

			/**
			 * @brief Checks if the value was provided.
			 * 
			 * This will always return true for required arguments.
			 * 
			 * @return True if a value was provided, false otherwise.
			*/
			explicit operator bool() const noexcept
			{
				return this->has_value();
			};

			/**
			 * @brief Checks if the stored value matches a particular type.
			 *
			 * @tparam T The type to check if this is storing.
			 * @return True if held type matches the given type, false otherwise.
			*/
			template <typename T>
			bool is_type() const noexcept
			{
				return this->value_.type().hash_code() == typeid(T).hash_code();
			};

			/**
			 * @brief Gets the stored value.
			 * 
			 * If `T` is not the stored value's type OR `has_value()`
			 * returns false, this will fail, throwing an exception.
			 * 
			 * Use try_get() to attempt to get the stored value without
			 * exception throwing behavior on failure.
			 * 
			 * @tparam T Type of the stored value.
			 * 
			 * @return Returns const reference to the stored value.
			 * 
			 * @throws exception Thrown if `T` is not the stored value's type.
			*/
			template <typename T>
			const T& get() const
			{
				return std::any_cast<T>(this->value_);
			};

			/**
			 * @brief Attempts to get the stored value.
			 *
			 * @tparam T Type of the stored value.
			 *
			 * @return Pointer to the stored value if `T` is the active type, nullptr otherwise
			 * or if the value was not provided (ie. has_value() returns false).
			*/
			template <typename T>
			const T* try_get() const
			{
				try
				{
					return std::any_cast<T>(&this->value_);
				}
				catch (const exception& _exc)
				{
					return nullptr;
				};
			};

			/**
			 * @brief Constructs an empty parsed value.
			 * 
			 * This is equivalent to defining an optional argument and no
			 * value was specified by the parsed arguments.
			*/
			ParsedValue() = default;

			// Allow copy/move
			ParsedValue(const ParsedValue& other) = default;
			ParsedValue& operator=(const ParsedValue& other) = default;
			ParsedValue(ParsedValue&& other) noexcept = default;
			ParsedValue& operator=(ParsedValue&& other) noexcept = default;

		private:

			// Allow complete access
			friend ArgumentParser;

			/**
			 * @brief Constructs the parsed value by copy.
			 * @tparam T Type to store.
			 * @param _value Value to copy.
			*/
			template <typename T>
			ParsedValue(const T& _value) :
				value_(_value)
			{};

			/**
			 * @brief Constructs the parsed value by move.
			 * @tparam T Type to store.
			 * @param _value Value to move.
			*/
			template <typename T>
			ParsedValue(T&& _value) noexcept :
				value_(std::move(_value))
			{};

			/**
			 * @brief The actual value.
			*/
			std::any value_;

		};

		// Forward decl for ParseResult
		class ParsedArgument;

		/**
		 * @brief Result type returned after parsing arguments.
		*/
		class ParseResult
		{
		private:

			/**
			 * @brief Storage for information about a parsed argument.
			*/
			struct ParsedArgumentInfo
			{
				/**
				 * @brief Offset into the parsed values storage this argument's values
				 * begin at.
				*/
				size_t offset;

				/**
				 * @brief Number of values parsed for the argument.
				*/
				size_t count;
			};

		public:

			/**
			 * @brief Returns true if the program should exit. This will be set if the
			 * help option was specified or if an error occured. Invoke and check the `error()`
			 * function to determine if an error occured.
			 * 
			 * @return True if program should exit (or at least not execute code if this is
			 * some kind of internal command. False otherwise.
			*/
			bool should_exit() const
			{
				return this->should_exit_;
			};

			/**
			 * @brief Returns true if there was an error during parsing.
			 * @return True on failure to parse, false otherwise.
			*/
			bool error() const
			{
				return this->error_occured_;
			};

			/**
			 * @brief Gets the message that should be displayed to the user.
			 * 
			 * This will be an error message if an error occured or possibly contain
			 * the help message or a warning otherwise. This may be empty if the parser
			 * did not generate a message.
			 * 
			 * @return Message string if provided.
			*/
			std::string message() const noexcept
			{
				return this->message_;
			};

			/**
			 * @brief Gets the parsed value for an argument using its label.
			 * 
			 * If `_label` isn't found, this function will throw an exception.
			 * 
			 * If `_label` refers to an optional argument AND the argument was not
			 * found specified during parsing, this will return an empty ParsedValue
			 * object (ie. has_value() will be false) and will not throw an exception.
			 * 
			 * @param _label Label of the argument.
			 * @return Handle accessing the parsed argument.
			 * 
			 * @throws std::out_of_range Thrown if `_label` is not the label of an argument specified
			 * by the parser this came from.
			*/
			ParsedArgument get(const std::string& _label) const;


			/**
			 * @brief Constructs an empty parse result.
			*/
			ParseResult() = default;

			ParseResult(ParseResult&& other) noexcept = default;
			ParseResult& operator=(ParseResult&& other) noexcept = default;

		private:

			// Allow complete access
			friend ArgumentParser;

			/**
			 * @brief Sets the value of the next positional argument.
			 * 
			 * All positional arguments MUST be set before named arguments are specified.
			 * 
			 * @param _values Zero or more values parsed for this positional argument. Pass an empty span for
			 * optional positional arguments.
			 * 
			 * @param _label Optional label specified for the argument. This isn't required for positional arguments.
			*/
			void set_positional_argument(std::span<const ParsedValue> _values, std::string _label = std::string{});

			/**
			 * @brief Sets the value of a named argument.
			 *
			 * Make sure all positional arguments were set prior to adding named arguments.
			 * 
			 * @param _values Zero or more values parsed for this named argument.
			 * @param _label Label for the argument. This is required and must NOT be an empty string.
			*/
			void set_named_argument(std::span<const ParsedValue> _values, std::string _label);

			/**
			 * @brief Constructs a parse result without any of the parsed arguments specified.
			 * 
			 * @param _shouldExit Set to true if program (or whatever this is parsing arguments for)
			 * should stop execution due to the parse. This should be set if help text was specified.
			 * 
			 * @param _errorOccured Set to true if an error occured during parsing. Set to false otherwise.
			 * 
			 * @param _message The message that should be displayed due to the parse. This should be
			 * any error message if _errorOccured was specified as true. If no error occured this may
			 * still be used to provide the help text.
			*/
			ParseResult(bool _shouldExit, bool _errorOccured, std::string _message = std::string{}) :
				should_exit_(_shouldExit), error_occured_(_errorOccured), message_(std::move(_message))
			{};

			/**
			 * @brief Storage for parsed argument values.
			*/
			std::vector<ParsedValue> parsed_values_;

			/**
			 * @brief Storage for information about a parsed argument.
			 * 
			 * Positional arguments (even if they do have a name) will always be FIRST.
			 * Named arguments will be last.
			*/
			std::vector<ParsedArgumentInfo> parsed_arguments_;

			/**
			 * @brief Map for pairing a positional/named argument's label with a position in
			 * the `parsed_arguments_` storage.
			*/
			std::unordered_map<std::string, size_t> labelled_argument_positions_;

			/**
			 * @brief Number of positional arguments stored in `parsed_arguments_`.
			*/
			size_t num_positional_args_ = 0;

			/**
			 * @brief Message to print.
			 *
			 * If `error_occured_` is true then this will be the error message.
			 * Otherwise this will only be set if the help optional argument was
			 * parsed.
			*/
			std::string message_;

			/**
			 * @brief True if the program should exit. This will be set if the
			 * help option was specified or if an error occured. Check `error_occured_`
			 * to determine if an error occured.
			*/
			bool should_exit_;

			/**
			 * @brief True if an error occured during parsing. False otherwise.
			*/
			bool error_occured_;

			// No copy
			ParseResult(const ParseResult& other) = delete;
			ParseResult& operator=(const ParseResult& other) = delete;
		};

		/**
		 * @brief Type returned when accessing parsed arguments from the ParseResult type.
		*/
		class ParsedArgument
		{
		public:

			/**
			 * @brief Value type referred to when accessing the ParsedArgument's value(s).
			*/
			using value_type = ParsedValue;

			/**
			 * @brief Pointer type for the value_type. Always a pointer to const as this type acts as a view.
			*/
			using pointer = const value_type*;

			/**
			 * @brief Reference type for the value_type. Always a reference to const as this type acts as a view.
			*/
			using reference = const value_type&;

		private:

			/**
			 * @brief Type used to view into the argument's parsed values.
			*/
			using view_type = std::span<const ParsedValue>;

		public:
			
			/**
			 * @brief Iterator type used to view the argument's values.
			*/
			using iterator = view_type::iterator;
			
			/**
			 * @brief Returns an iterator to the first provided value.
			 * @return ParsedValue iterator.
			*/
			iterator begin() const noexcept
			{
				return this->values_.begin();
			};

			/**
			 * @brief Returns an iterator to the one past the last provided value.
			 * @return ParsedValue iterator.
			*/
			iterator end() const noexcept
			{
				return this->values_.end();
			};

			/**
			 * @brief Gets the number of values provided for this argument.
			 * @return Number of values.
			*/
			size_t value_count() const
			{
				return this->values_.size();
			};

			/**
			 * @brief Checks if any values were provided for this argument.
			 * @return True if at least one value was provided, false otherwise.
			*/
			bool has_value() const noexcept
			{
				return !this->values_.empty();
			};

			/**
			 * @brief Checks if any values were provided for this argument.
			 * @return True if at least one value was provided, false otherwise.
			*/
			explicit operator bool() const noexcept
			{
				return this->has_value();
			};

			/**
			 * @brief Gets the first value provided for this argument.
			 * @return Reference to a parsed value.
			 * @throws std::out_of_range Thrown if there were no values provided for this argument.
			*/
			reference value() const
			{
				return this->values_.front();
			};

			/**
			 * @brief Gets the first value provided for this argument.
			 * @return Reference to a parsed value.
			 * @throws std::out_of_range Thrown if there were no values provided for this argument.
			*/
			reference operator*() const
			{
				return this->value();
			};

			/**
			 * @brief Gets a pointer to the first value provided for this argument.
			 * 
			 * NOTE: This will never return nullptr.
			 * 
			 * @return Pointer to a parsed value. Never nullptr.
			 * 
			 * @throws std::out_of_range Thrown if there were no values provided for this argument.
			*/
			pointer operator->() const
			{
				return &this->value();
			};

		private:

			// Allow full access
			friend ParseResult;

			/**
			 * @brief Constructs the parsed argument as having zero or more parsed values.
			 * @param _values Parsed values for the argument.
			*/
			explicit ParsedArgument(view_type _values) :
				values_(_values)
			{};

			/**
			 * @brief View into the parsed values for this argument.
			*/
			view_type values_;
		};

		/**
		 * @brief Parses an array of arguments as they would be provided to an
		 * application's main function.
		 *
		 * This function assumes that the first argument is NOT the path used to
		 * execute a file. Normally you get that path as the first argument from
		 * the C/C++ main() function. If you are certain that the first argument will
		 * not be this path then use this function.
		 * 
		 * @param _args Span of argument strings.
		 *
		 * @return Parse result containing information about the parse.
		*/
		ParseResult parse_args_no_execute_filename(std::span<const std::string_view> _args);

		/**
		 * @brief Parses an array of arguments as they would be provided to an
		 * application's main function.
		 * 
		 * If the first argument is the path used for the executed file, it will be ignored.
		 * 
		 * This is not 100% perfect and will result in a false positive if this value was actually
		 * provided by the user and not from some OS functionality.
		 * 
		 * You can use the function `parse_args_no_execute_filename` if you know for a fact that
		 * the first argument will not be the executed file path.
		 *
		 * @param _args Span of argument strings.
		 * 
		 * @return Parse result containing information about the parse.
		*/
		ParseResult parse_args(std::span<const std::string_view> _args);

		/**
		 * @brief Parses an array of arguments as they would be provided to an
		 * application's main function.
		 * 
		 * If the first argument is the path used for the executed file, it will be ignored.
		 * 
		 * This is not 100% perfect and will result in a false positive if this value was actually
		 * provided by the user and not from some OS functionality.
		 * 
		 * You can use the function `parse_args_no_execute_filename` if you know for a fact that
		 * the first argument will not be the executed file path.
		 *
		 * @param _nargs Number of arguments in _vargs.
		 * @param _vargs Pointer to an array of c-strings. Doesn't have to end with a null pointer.
		 * 
		 * @return Parse result containing information about the parse.
		*/
		ParseResult parse_args(int _nargs, const char* const* _vargs);




		/**
		 * @brief Provides a handle to an argument definition and allows configuring it.
		 * 
		 * WARNING: All definition handles will be invalidated if the ArgumentParser they
		 * come from is moved or destroyed. To help avoid such a situation this type cannot
		 * be "saved" and instead must be used entirely right after creation.
		*/
		class ArgumentDefinitionHandle
		{
		private:

			/**
			 * @brief Gets the argument definition this handle refers to.
			 * @return Argument definition.
			*/
			ArgumentDefinition& get_definition() const;

		public:

			/**
			 * @brief Alias of this type.
			*/
			using self_type = ArgumentDefinitionHandle;

			/**
			 * @brief Sets a label used to refer to an argument.
			 * 
			 * Does not affect whether this argument is positional or named.
			 * 
			 * @param _label Label text.
			 * @return This definition handle.
			*/
			self_type& set_label(const std::string_view _label)
			{
				auto& _def = this->get_definition();
				_def.label = _label;
				return *this;
			};

			/**
			 * @brief Sets the optional description text used for the help message.
			 * @param _description Description text.
			 * @return This definition handle.
			*/
			self_type& set_description(const std::string_view _description)
			{
				auto& _def = this->get_definition();
				_def.description = _description;
				return *this;
			};

			/**
			 * @brief Adds a name to the argument. Multiple names can be used and
			 * will act as aliases of the argument.
			 * 
			 * If the name begins with "-" or "--" this will make the argument optional.
			 * All previously added names must also begin with "-" or "--" or an error
			 * will be generated.
			 * 
			 * If the argument is optional already the name must start with "-" or "--".
			 * 
			 * Using this will make the argument a named argument if it wasn't already.
			 * 
			 * @param _name Name text.
			 * @return This definition handle.
			*/
			self_type& add_name(std::string_view _name);






			~ArgumentDefinitionHandle() = default;

		private:

			// Allow full access.
			friend ArgumentParser;

			/**
			 * @brief Constructs a definition handle to a particular definition in a parser.
			 * @param _parser Argument parser this handle looks into.
			 * @param _index The index of the definition in the parser.
			*/
			explicit ArgumentDefinitionHandle(ArgumentParser& _parser, size_t _index) :
				parser_(&_parser), index_(_index)
			{};

			// Allows ArgumentParser to move the handle but no one else.
			ArgumentDefinitionHandle(ArgumentDefinitionHandle&& other) noexcept = default;

			/**
			 * @brief The argument parser this handle points into.
			*/
			ArgumentParser* parser_;

			/**
			 * @brief Index of the defintion in the parser.
			*/
			size_t index_;

			// No copy and no move assign
			ArgumentDefinitionHandle(const ArgumentDefinitionHandle& other) = delete;
			ArgumentDefinitionHandle& operator=(const ArgumentDefinitionHandle& other) = delete;
			ArgumentDefinitionHandle& operator=(ArgumentDefinitionHandle&& other) noexcept = delete;
		};

		/**
		 * @brief Adds an argument to the parser.
		 * 
		 * @param _label Label used to refer to the argument in code only.
		 * @param _description Optional description printed as a part of the help text.
		 * 
		 * @return Argument definition handle which can be used to configure the new argument.
		*/
		ArgumentDefinitionHandle add_argument(std::string_view _label, std::string_view _description = std::string_view{});

		/**
		 * @brief Constructs an empty argument parser.
		 * @param _name Name of the program this is parsing for.
		 * @param _description Description text used to describe the program's overall function.
		*/
		explicit ArgumentParser(const std::string& _name, const std::string& _description);

		/**
		 * @brief Constructs an empty argument parser.
		 * @param _name Name of the program this is parsing for.
		*/
		explicit ArgumentParser(const std::string& _name);

	private:

		// Allow argument definition handle to access what it needs
		friend ArgumentDefinitionHandle;

		/**
		 * @brief Defines default arguments (like -h).
		*/
		void define_default_arguments();

		/**
		 * @brief Resolves the defined argument's metalabel values.
		*/
		void resolve_argument_metalabels();

		/**
		 * @brief Generates the help text printed when the help option is provided.
		 * @return Text with help message.
		*/
		std::string generate_help_text();

		/**
		 * @brief The name of the program.
		*/
		std::string name_;

		/**
		 * @brief Optional description of the program's overall function.
		*/
		std::string description_;

		/**
		 * @brief Storage for the argument definitions for this parser.
		*/
		std::vector<ArgumentDefinition> argument_definitions_;
	};
};