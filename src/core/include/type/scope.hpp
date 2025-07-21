#ifndef PLIB_CORE_TYPE_SCOPE_HPP
#define PLIB_CORE_TYPE_SCOPE_HPP


namespace plib::core::type{



        class auto_scope_impl
        {
            std::function<void()> end_lambda_;

        public:

            auto_scope_impl(std::function<void()> _lambda)
                : end_lambda_(std::move(_lambda))
            {
                assert(end_lambda_);
            }

            ~auto_scope_impl()
            {
                    end_lambda_();

            }
        };

        using auto_scope_sptr = std::shared_ptr<auto_scope_impl>;


        class auto_scope_bool_impl
        {
            std::function<void(const bool)> end_lambda_;

            bool success_ = false;

        public:

            void set_success()
            {
                success_ = true;
            }

            auto_scope_bool_impl(std::function<void(const bool)> _lambda)
                : end_lambda_(std::move(_lambda))
            {
                assert(end_lambda_);
            }

            ~auto_scope_bool_impl()
            {
                
                    end_lambda_(success_);
             
            }
        };

     
} // namespace plib::core::type

#endif // PLIB_CORE_TYPE_SCOPE_HPP